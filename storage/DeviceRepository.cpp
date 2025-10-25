/**
 * @file DeviceRepository.cpp
 * @brief Реализация методов доступа к БД для устройств (SQLite).
 */

#include "Repositories.h"

#if __has_include("ReserveDevice.h")
#include "ReserveDevice.h"   ///< Для is_reserve через dynamic_cast
#elif __has_include("../ReserveDevice.h")
#include "../ReserveDevice.h"
#endif

 // Подключение C-API SQLite (для IntelliSense это важно)
extern "C" {
#if __has_include(<sqlite3.h>)
#include <sqlite3.h>
#else
#include "sqlite3.h"
#endif
}

#include <stdexcept>
#include <vector>
#include <string>

/**
 * @brief INSERT OR UPDATE по ключу address.
 * @details Привязки:
 *  1) address    -> INTEGER (bind_int64)
 *  2) name       -> TEXT    (c_str() + SQLITE_TRANSIENT)
 *  3) priority   -> INTEGER
 *  4) is_faulty  -> INTEGER 0/1
 *  5) is_reserve -> INTEGER 0/1 (через dynamic_cast к ReserveDevice)
 */
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
        throw std::runtime_error(std::string("prepare upsert device failed: ") + sqlite3_errmsg(db));
    }

    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(d.address()));
    sqlite3_bind_text(stmt, 2, d.name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, static_cast<int>(d.priority()));
    sqlite3_bind_int(stmt, 4, d.isFaulty() ? 1 : 0);

#if __has_include("ReserveDevice.h") || __has_include("../ReserveDevice.h")
    const bool isReserve = (dynamic_cast<const ReserveDevice*>(&d) != nullptr);
#else
    const bool isReserve = false;
#endif
    sqlite3_bind_int(stmt, 5, isReserve ? 1 : 0);

    const int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::string msg = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error("upsert device failed: " + msg);
    }
    sqlite3_finalize(stmt);
}

/**
 * @brief Получить все адреса устройств.
 * @return Вектор адресов.
 */
std::vector<Device::Address> DeviceRepository::listAddresses() {
    static constexpr const char* SQL = "SELECT address FROM devices";

    sqlite3* db = st_.handle();
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("prepare list failed: ") + sqlite3_errmsg(db));
    }

    std::vector<Device::Address> v;
    for (;;) {
        const int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            const sqlite3_int64 addr64 = sqlite3_column_int64(stmt, 0);
            v.push_back(static_cast<Device::Address>(addr64));
        }
        else if (rc == SQLITE_DONE) {
            break;
        }
        else {
            std::string msg = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw std::runtime_error("listAddresses step failed: " + msg);
        }
    }
    sqlite3_finalize(stmt);
    return v;
}

