/**
 * @file DeviceRepository.cpp
 * @brief Реализация методов Repositories
 */
#include "Repositories.h"



#include "../ReserveDevice.h"

extern "C" {
#include <sqlite3.h>
}

#include <stdexcept>
#include <vector>

void DeviceRepository::upsert(const Device& d) {
    static constexpr const char* SQL =
        "INSERT INTO devices(address,name,priority,is_faulty,is_reserve) "
        "VALUES(?,?,?,?,?) "
        "ON CONFLICT(address) DO UPDATE SET "
        "name=excluded.name, priority=excluded.priority, "
        "is_faulty=excluded.is_faulty, is_reserve=excluded.is_reserve;";

    sqlite3* db = st_.handle();
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare upsert device failed");
    }

    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(d.address()));
    sqlite3_bind_text(stmt, 2, d.name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, static_cast<int>(d.priority()));
    sqlite3_bind_int(stmt, 4, d.isFaulty() ? 1 : 0);

    int is_reserve = 0;
#if __has_include("ReserveDevice.h") || __has_include("../ReserveDevice.h")
    if (dynamic_cast<const ReserveDevice*>(&d) != nullptr) {
        is_reserve = 1;
    }
#endif
    sqlite3_bind_int(stmt, 5, is_reserve);

    const int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("upsert device failed");
    }
    sqlite3_finalize(stmt);
}

std::vector<Device::Address> DeviceRepository::listAddresses() const {
    static constexpr const char* SQL = "SELECT address FROM devices";

    sqlite3* db = st_.handle();
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare list failed");
    }

    std::vector<Device::Address> v;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_int64 addr64 = sqlite3_column_int64(stmt, 0);
        v.push_back(static_cast<Device::Address>(addr64));
    }
    sqlite3_finalize(stmt);
    return v;
}