/**
 * @file Engine.cpp
 * @brief Реализация фасада ESM
 */

#include "Engine.h"
extern "C" {
#include <sqlite3.h>
}
#include <stdexcept>

Engine::Engine(const std::string& db_path)
    : storage_(db_path), dev_repo_(storage_), worker_repo_(storage_) {
}

void Engine::migrate() {
    
    storage_.exec(
        "CREATE TABLE IF NOT EXISTS devices("
        " address INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " priority INTEGER NOT NULL CHECK(priority IN (0,1,2)),"
        " is_faulty INTEGER NOT NULL CHECK(is_faulty IN (0,1)),"
        " is_reserve INTEGER NOT NULL CHECK(is_reserve IN (0,1)),"
        " updated_at INTEGER NOT NULL DEFAULT (strftime('%s','now')));"
    );
    storage_.exec(
        "CREATE TABLE IF NOT EXISTS workers("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " name TEXT NOT NULL,"
        " max_jobs INTEGER NOT NULL DEFAULT 1,"
        " skill TEXT);"
    );
    storage_.exec(
        "CREATE TABLE IF NOT EXISTS jobs("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " device_address INTEGER NOT NULL REFERENCES devices(address) ON DELETE CASCADE,"
        " worker_id INTEGER REFERENCES workers(id) ON DELETE SET NULL,"
        " fault TEXT NOT NULL,"
        " status INTEGER NOT NULL CHECK(status IN (0,1,2,3)),"
        " created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
        " started_at INTEGER, finished_at INTEGER);"
    );
    storage_.exec("CREATE INDEX IF NOT EXISTS idx_jobs_device  ON jobs(device_address);");
    storage_.exec("CREATE INDEX IF NOT EXISTS idx_jobs_status  ON jobs(status);");
}

std::int64_t Engine::add_worker(const std::string& name, int max_jobs, const std::string& skill) {
    return worker_repo_.insert(name, max_jobs, skill);
}
std::vector<WorkerFlat> Engine::list_workers() const {
    auto rows = worker_repo_.list();
    std::vector<WorkerFlat> out;
    out.reserve(rows.size());
    for (auto& r : rows) out.push_back({ r.id, r.name, r.max_jobs, r.skill });
    return out;
}

void Engine::upsert_device(const Device& d) {
    dev_repo_.upsert(d);
}

std::vector<DeviceRow> Engine::list_devices() const {

    auto addrs = dev_repo_.listAddresses();
    std::vector<DeviceRow> out;
    out.reserve(addrs.size());

    sqlite3* db = storage_.handle(); 
    sqlite3_stmt* stmt = nullptr;
    const char* SQL =
        "SELECT address,name,priority,is_faulty,is_reserve "
        "FROM devices WHERE address=?1;";
    for (auto a : addrs) {
        if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error(std::string("prepare list_devices failed: ") + sqlite3_errmsg(db));
        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(a));
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("device not found");
        }
        DeviceRow r;
        r.address = static_cast<std::uint32_t>(sqlite3_column_int64(stmt, 0));
        r.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.priority = static_cast<ServicePriority>(sqlite3_column_int(stmt, 2));
        r.is_faulty = sqlite3_column_int(stmt, 3) != 0;
        r.is_reserve = sqlite3_column_int(stmt, 4) != 0;
        out.push_back(std::move(r));
        sqlite3_finalize(stmt);
    }
    return out;
}

std::int64_t Engine::breakdown(std::uint32_t address, const std::string& fault) {
    sqlite3* db = storage_.handle();
    sqlite3_stmt* stmt = nullptr;

    //  Создать/обновить запись об устройстве как неисправной
    storage_.exec("BEGIN IMMEDIATE;");
    {
        const char* UPSERT =
            "INSERT INTO devices(address,name,priority,is_faulty,is_reserve) "
            "VALUES(?1,'Unknown',2,1,0) "
            "ON CONFLICT(address) DO UPDATE SET is_faulty=1, updated_at=strftime('%s','now');";
        if (sqlite3_prepare_v2(db, UPSERT, -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error(std::string("prepare devices upsert failed: ") + sqlite3_errmsg(db));
        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(address));
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::string msg = sqlite3_errmsg(db); sqlite3_finalize(stmt); storage_.rollback();
            throw std::runtime_error("devices upsert failed: " + msg);
        }
        sqlite3_finalize(stmt);
    }
    //  Создать job
    std::int64_t job_id = 0;
    {
        const char* INS = "INSERT INTO jobs(device_address,fault,status) VALUES(?1,?2,0);";
        if (sqlite3_prepare_v2(db, INS, -1, &stmt, nullptr) != SQLITE_OK) {
            storage_.rollback();
            throw std::runtime_error(std::string("prepare insert job failed: ") + sqlite3_errmsg(db));
        }
        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(address));
        sqlite3_bind_text(stmt, 2, fault.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::string msg = sqlite3_errmsg(db); sqlite3_finalize(stmt); storage_.rollback();
            throw std::runtime_error("insert job failed: " + msg);
        }
        sqlite3_finalize(stmt);
        job_id = static_cast<std::int64_t>(sqlite3_last_insert_rowid(db));
    }
    storage_.commit();
    return job_id;
}

void Engine::start_repair(std::int64_t job_id, std::int64_t worker_id) {
    sqlite3* db = storage_.handle();
    sqlite3_stmt* stmt = nullptr;
    const char* SQL =
        "UPDATE jobs SET status=1, worker_id=?2, started_at=strftime('%s','now') "
        "WHERE id=?1 AND status=0;";
    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(std::string("prepare start_repair failed: ") + sqlite3_errmsg(db));
    sqlite3_bind_int64(stmt, 1, job_id);
    sqlite3_bind_int64(stmt, 2, worker_id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string msg = sqlite3_errmsg(db); sqlite3_finalize(stmt);
        throw std::runtime_error("start_repair failed: " + msg);
    }
    sqlite3_finalize(stmt);
}

void Engine::finish_repair(std::int64_t job_id, std::uint64_t uptime_after_sec) {
    sqlite3* db = storage_.handle();
    sqlite3_stmt* stmt = nullptr;

    storage_.exec("BEGIN IMMEDIATE;");
    // узнать адрес устройства по job_id
    std::uint32_t address = 0;
    {
        const char* SEL = "SELECT device_address FROM jobs WHERE id=?1;";
        if (sqlite3_prepare_v2(db, SEL, -1, &stmt, nullptr) != SQLITE_OK) {
            storage_.rollback();
            throw std::runtime_error(std::string("prepare select job failed: ") + sqlite3_errmsg(db));
        }
        sqlite3_bind_int64(stmt, 1, job_id);
        if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); storage_.rollback(); throw std::runtime_error("job not found"); }
        address = static_cast<std::uint32_t>(sqlite3_column_int64(stmt, 0));
        sqlite3_finalize(stmt);
    }
    // Работу в выполненную
    {
        const char* UPD = "UPDATE jobs SET status=2, finished_at=strftime('%s','now') WHERE id=?1;";
        if (sqlite3_prepare_v2(db, UPD, -1, &stmt, nullptr) != SQLITE_OK) {
            storage_.rollback();
            throw std::runtime_error(std::string("prepare finish job failed: ") + sqlite3_errmsg(db));
        }
        sqlite3_bind_int64(stmt, 1, job_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) { std::string msg = sqlite3_errmsg(db); sqlite3_finalize(stmt); storage_.rollback(); throw std::runtime_error("finish job failed: " + msg); }
        sqlite3_finalize(stmt);
    }
    //отметим device как исправный
    {
        const char* DEV =
            "UPDATE devices SET is_faulty=0, updated_at=strftime('%s','now') WHERE address=?1;";
        if (sqlite3_prepare_v2(db, DEV, -1, &stmt, nullptr) != SQLITE_OK) {
            storage_.rollback();
            throw std::runtime_error(std::string("prepare device update failed: ") + sqlite3_errmsg(db));
        }
        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(address));
        if (sqlite3_step(stmt) != SQLITE_DONE) { std::string msg = sqlite3_errmsg(db); sqlite3_finalize(stmt); storage_.rollback(); throw std::runtime_error("device update failed: " + msg); }
        sqlite3_finalize(stmt);
    }
    storage_.commit();

    (void)uptime_after_sec;
}

std::vector<JobRow> Engine::list_jobs(std::optional<JobStatus> status) const {
    sqlite3* db = storage_.handle();
    sqlite3_stmt* stmt = nullptr;

    const char* SQL_ALL =
        "SELECT id,device_address,worker_id,fault,status,created_at,started_at,finished_at "
        "FROM jobs ORDER BY id;";
    const char* SQL_FLT =
        "SELECT id,device_address,worker_id,fault,status,created_at,started_at,finished_at "
        "FROM jobs WHERE status=?1 ORDER BY id;";

    if (sqlite3_prepare_v2(db, status ? SQL_FLT : SQL_ALL, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(std::string("prepare list_jobs failed: ") + sqlite3_errmsg(db));
    if (status) sqlite3_bind_int(stmt, 1, static_cast<int>(*status));

    std::vector<JobRow> out;
    while (true) {
        const int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            JobRow r{};
            r.id = sqlite3_column_int64(stmt, 0);
            r.device_address = static_cast<std::uint32_t>(sqlite3_column_int64(stmt, 1));
            if (sqlite3_column_type(stmt, 2) == SQLITE_NULL) r.worker_id = std::nullopt;
            else r.worker_id = sqlite3_column_int64(stmt, 2);
            r.fault = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            r.status = static_cast<JobStatus>(sqlite3_column_int(stmt, 4));
            r.created_at = sqlite3_column_int64(stmt, 5);
            r.started_at = (sqlite3_column_type(stmt, 6) == SQLITE_NULL) ? std::nullopt
                : std::optional<std::int64_t>(sqlite3_column_int64(stmt, 6));
            r.finished_at = (sqlite3_column_type(stmt, 7) == SQLITE_NULL) ? std::nullopt
                : std::optional<std::int64_t>(sqlite3_column_int64(stmt, 7));
            out.push_back(std::move(r));
        }
        else if (rc == SQLITE_DONE) break;
        else { std::string msg = sqlite3_errmsg(db); sqlite3_finalize(stmt); throw std::runtime_error("list_jobs failed: " + msg); }
    }
    sqlite3_finalize(stmt);
    return out;
}
