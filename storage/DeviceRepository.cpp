#include "Repositories.h"   
#include <stdexcept>  
#include <vector> 

void DeviceRepository::upsert(const Device& d) {
    const char* sql =
        "INSERT INTO devices(address,name,priority,is_faulty,is_reserve) "
        "VALUES(?,?,?,?,?) "
        "ON CONFLICT(address) DO UPDATE SET "
        "name=excluded.name, priority=excluded.priority, "
        "is_faulty=excluded.is_faulty, is_reserve=excluded.is_reserve;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3* db = st_.handle();

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare upsert device failed");
    }

    sqlite3_bind_int(stmt, 1, static_cast<int>(d.address()));
    sqlite3_bind_text(stmt, 2, d.name(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, static_cast<int>(d.priority()));
    sqlite3_bind_int(stmt, 4, d.isFaulty() ? 1 : 0);
    sqlite3_bind_int(stmt, 5, d.isReserve() ? 1 : 0);

    const int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("upsert device failed");
    }
    sqlite3_finalize(stmt);
}

std::vector<Device::Address> DeviceRepository::listAddresses() {
    const char* sql = "SELECT address FROM devices";

    sqlite3_stmt* stmt = nullptr;
    sqlite3* db = st_.handle();

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare list failed");
    }

    std::vector<Device::Address> v;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        v.push_back(static_cast<Device::Address>(sqlite3_column_int(stmt, 0)));
    }
    sqlite3_finalize(stmt);
    return v;
}
