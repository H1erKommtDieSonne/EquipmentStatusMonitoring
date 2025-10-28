/**
 * @file WorkerRepository.cpp
 * @brief Реализация методов WorkerRepository
 */
#include "WorkerRepository.h"

extern "C" {
#include <sqlite3.h>
}

#include <stdexcept>

std::int64_t WorkerRepository::insert(const std::string& name, int max_jobs, const std::string& skill) {
    static constexpr const char* SQL =
        "INSERT INTO workers(name,max_jobs,skill) VALUES(?,?,?);";

    sqlite3* db = st_.handle();
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(std::string("prepare insert worker failed: ") + sqlite3_errmsg(db));

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, max_jobs);
    sqlite3_bind_text(stmt, 3, skill.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string msg = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error("insert worker failed: " + msg);
    }
    sqlite3_finalize(stmt);
    return static_cast<std::int64_t>(sqlite3_last_insert_rowid(db));
}

std::vector<WorkerRow> WorkerRepository::list() const {
    static constexpr const char* SQL =
        "SELECT id,name,max_jobs,skill FROM workers ORDER BY id;";

    sqlite3* db = st_.handle();
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(std::string("prepare list workers failed: ") + sqlite3_errmsg(db));

    std::vector<WorkerRow> out;
    while (true) {
        const int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            WorkerRow w;
            w.id = sqlite3_column_int64(stmt, 0);
            w.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            w.max_jobs = sqlite3_column_int(stmt, 2);
            const unsigned char* txt = sqlite3_column_text(stmt, 3);
            w.skill = txt ? reinterpret_cast<const char*>(txt) : "";
            out.push_back(std::move(w));
        }
        else if (rc == SQLITE_DONE) {
            break;
        }
        else {
            std::string msg = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw std::runtime_error("list workers failed: " + msg);
        }
    }
    sqlite3_finalize(stmt);
    return out;
}
