/**
 * @file main.cpp
 * @brief Набор тестов
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <functional>
#include <cstdint>
#include <stdexcept>

#include "storage/SqliteStorage.h"
#include "storage/WorkerRepository.h"
#include "storage/Repositories.h"

#include "FaultyDevice.h"
#include "FaultyDeviceEx.h"
#include "ReserveDevice.h"
#include "HealthyDevice.h"
#include "Device.h"
#include "ServicePriority.h"

#include "esm/Engine.h"

extern "C" {
#include <sqlite3.h>
}

using FD = FaultyDevice;

//FaultyDevice

TEST(FaultyDevice_Ip, RoundTripBasic) {
    uint32_t v = FaultyDevice::ipv4_to_u32("10.0.0.5");
    EXPECT_EQ(FaultyDevice::u32_to_ipv4(v), "10.0.0.5");

    v = FaultyDevice::ipv4_to_u32("192.168.0.1");
    EXPECT_EQ(FaultyDevice::u32_to_ipv4(v), "192.168.0.1");
}

TEST(FaultyDevice_Ip, RoundTripEdges) {
    EXPECT_EQ(FaultyDevice::u32_to_ipv4(FaultyDevice::ipv4_to_u32("0.0.0.0")), "0.0.0.0");
    EXPECT_EQ(FaultyDevice::u32_to_ipv4(FaultyDevice::ipv4_to_u32("255.255.255.255")), "255.255.255.255");
}

TEST(FaultyDevice_Ip, RoundTripSamples) {
    const char* samples[] = { "1.2.3.4", "172.16.5.10", "8.8.8.8" };
    for (auto s : samples) {
        uint32_t x = FaultyDevice::ipv4_to_u32(s);
        EXPECT_EQ(FaultyDevice::u32_to_ipv4(x), std::string(s));
    }
}

TEST(FaultyDevice_Ip, InvalidFormat) {
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0.1.2"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10-0-0-5"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10,0,0,5"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("a.b.c.d"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.a.0.5"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32(""), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("   "), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0.5abc"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.-1.1"), std::invalid_argument);
}

TEST(FaultyDevice_Ip, OutOfRangeOctet) {
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("256.0.0.1"), std::out_of_range);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.300.0.1"), std::out_of_range);
}

//FaultyDevice2

TEST(FaultyDevice_Order, PriorityAndTies) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.5");
    FaultyDevice a("A", addr, ServicePriority::High, "X");
    FaultyDevice b("B", addr, ServicePriority::Low, "Y");
    FaultyDevice c("C", addr, ServicePriority::None, "Z");

    EXPECT_GT(a, b);
    EXPECT_GT(b, c);

    FaultyDevice d("D", addr, ServicePriority::High, "Q");
    EXPECT_TRUE(a == d);
}

TEST(FaultyDevice_Order, SortByPriorityDesc) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.5");
    FaultyDevice a("A", addr, ServicePriority::High, "X");
    FaultyDevice b("B", addr, ServicePriority::Low, "Y");
    FaultyDevice c("C", addr, ServicePriority::None, "Z");

    std::vector<FaultyDevice> v{ b, c, a };
    std::sort(v.begin(), v.end(), std::greater<>());
    EXPECT_EQ(v.front().priority(), ServicePriority::High);
    EXPECT_EQ(v.back().priority(), ServicePriority::None);
}

//FaultyDevice3

TEST(FaultyDevice_Address, StoredAndUnique) {
    std::vector<std::string> ips = { "10.0.0.5", "10.0.0.6", "192.168.0.1", "8.8.8.8" };

    std::vector<FaultyDevice> devices;
    devices.reserve(ips.size());

    for (size_t i = 0; i < ips.size(); ++i) {
        uint32_t addr = FaultyDevice::ipv4_to_u32(ips[i]);
        devices.emplace_back("Dev#" + std::to_string(i), addr, ServicePriority::Low, "test");
    }

    for (size_t i = 0; i < devices.size(); ++i) {
        EXPECT_EQ(FaultyDevice::u32_to_ipv4(devices[i].address()), ips[i]);
    }

    std::unordered_set<uint32_t> seen;
    for (auto& d : devices) {
        auto [it, inserted] = seen.insert(d.address());
        EXPECT_TRUE(inserted);
    }
    EXPECT_EQ(seen.size(), devices.size());
}

//Healthy/Reserve

TEST(HealthyDevice_Basics, StoresFieldsAndAllowsNonePriority) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.10");
    HealthyDevice h("H", addr, ServicePriority::None, 120u);

    EXPECT_EQ(h.name(), "H");
    EXPECT_EQ(h.address(), addr);
    EXPECT_EQ(h.priority(), ServicePriority::None);
}

TEST(ReserveDevice_Basics, StandbyAndUptimeTracked) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.12");
    ReserveDevice r("R", addr, ServicePriority::Low,  300u,  24u);

    EXPECT_EQ(r.name(), "R");
    EXPECT_EQ(r.address(), addr);
    EXPECT_EQ(r.priority(), ServicePriority::Low);
}

TEST(FaultyDevice_Compare, StrictWeakOrderingConsistentWithSort) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.20");
    FaultyDevice high("H", addr, ServicePriority::High, "h");
    FaultyDevice low("L", addr, ServicePriority::Low, "l");
    FaultyDevice none("N", addr, ServicePriority::None, "n");

    std::vector<FaultyDevice> v{ none, low, high };
    std::sort(v.begin(), v.end(), std::greater<>());

    EXPECT_GT(high, low);
    EXPECT_GT(low, none);
    EXPECT_FALSE(high < none);

    FaultyDevice high2("H2", addr, ServicePriority::High, "h2");
    EXPECT_TRUE(high == high2 || (!(high < high2) && !(high2 < high)));
}

//SQLite

/// \brief Создать таблицы devices и workers
static void create_schema(SqliteStorage& st) {
    st.exec(
        "CREATE TABLE IF NOT EXISTS devices("
        " address INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " priority INTEGER NOT NULL,"
        " is_faulty INTEGER NOT NULL,"
        " is_reserve INTEGER NOT NULL);"
    );
    st.exec(
        "CREATE TABLE IF NOT EXISTS workers("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " name TEXT NOT NULL,"
        " max_jobs INTEGER NOT NULL DEFAULT 1,"
        " skill TEXT);"
    );
}

//SQLite tests

TEST(Sqlite_Schema, CreateTables) {
    SqliteStorage st(":memory:");
    create_schema(st);

    auto exists = [&](const char* name) {
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(st.handle(),
            "SELECT name FROM sqlite_master WHERE type='table' AND name=?1", -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
        bool ok = (sqlite3_step(stmt) == SQLITE_ROW);
        sqlite3_finalize(stmt);
        return ok;
        };

    EXPECT_TRUE(exists("devices"));
    EXPECT_TRUE(exists("workers"));
}

TEST(Repository_Seed_DevicesAndWorkers, UpsertAndList) {
    SqliteStorage st(":memory:");
    create_schema(st);

    DeviceRepository dr(st);
    WorkerRepository wr(st);

    auto mk = [](const char* s) { return FD::ipv4_to_u32(s); };

    HealthyDevice  dH("Router-1", mk("10.0.0.10"), ServicePriority::None, 120);
    ReserveDevice  dR("UPS-Backup", mk("10.0.0.11"), ServicePriority::Low, 3600, 86400);
    FaultyDeviceEx dF("Switch-3", mk("10.0.0.12"), ServicePriority::High, "fan failure");

    dr.upsert(dH); dr.upsert(dR); dr.upsert(dF);

    auto addrs = dr.listAddresses();
    ASSERT_EQ(addrs.size(), 3u);
    std::sort(addrs.begin(), addrs.end());
    EXPECT_EQ(addrs[0], mk("10.0.0.10"));
    EXPECT_EQ(addrs[1], mk("10.0.0.11"));
    EXPECT_EQ(addrs[2], mk("10.0.0.12"));

    auto id1 = wr.insert("Иван", 2, "электрика");
    auto id2 = wr.insert("Мария", 1, "сети");
    auto id3 = wr.insert("Роберт", 3, "универсал");
    auto workers = wr.list();

    EXPECT_EQ(workers.size(), 3u);
    EXPECT_NE(id1, 0);
    EXPECT_NE(id2, 0);
    EXPECT_NE(id3, 0);
    EXPECT_EQ(workers[0].name, "Иван");
}

TEST(Repository_IsReserveFlag, SetByDynamicCast) {
    SqliteStorage st(":memory:"); create_schema(st);
    DeviceRepository dr(st);

    auto addr = FD::ipv4_to_u32("10.0.0.13");
    ReserveDevice r("R1", addr, ServicePriority::Low, 10, 60);
    dr.upsert(r);

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(st.handle(),
        "SELECT is_reserve, is_faulty FROM devices WHERE address=?1", -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(addr));
    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    EXPECT_EQ(sqlite3_column_int(stmt, 0), 1);
    EXPECT_EQ(sqlite3_column_int(stmt, 1), 0);
    sqlite3_finalize(stmt);
}

TEST(Repository_UpsertUpdate, ChangesPersist) {
    SqliteStorage st(":memory:"); create_schema(st);
    DeviceRepository dr(st);

    auto addr = FD::ipv4_to_u32("10.0.0.14");
    HealthyDevice  h("H", addr, ServicePriority::Low, 1);
    FaultyDeviceEx f("H", addr, ServicePriority::High, "boom");

    dr.upsert(h);
    dr.upsert(f);

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(st.handle(),
        "SELECT priority, is_faulty FROM devices WHERE address=?1", -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(addr));
    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    EXPECT_EQ(sqlite3_column_int(stmt, 0), static_cast<int>(ServicePriority::High));
    EXPECT_EQ(sqlite3_column_int(stmt, 1), 1);
    sqlite3_finalize(stmt);
}

//Engine tests

TEST(Engine_EndToEnd, Breakdown_Assign_Finish) {
    Engine eng(":memory:");
    eng.migrate();

    
    auto wid = eng.add_worker("Мария", 1, "сети");
    ASSERT_GT(wid, 0);

    
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.50");
    HealthyDevice h("Edge-Switch", addr, ServicePriority::Low, 100);
    eng.upsert_device(h);

    
    auto job = eng.breakdown(addr, "fan failure");
    ASSERT_GT(job, 0);

    
    EXPECT_NO_THROW(eng.start_repair(job, wid));

    
    EXPECT_NO_THROW(eng.finish_repair(job, 200));

    
    auto jobs = eng.list_jobs(std::nullopt);
    ASSERT_FALSE(jobs.empty());
    EXPECT_EQ(jobs.back().status, JobStatus::Done);

    
    auto devices = eng.list_devices();
    auto it = std::find_if(devices.begin(), devices.end(), [&](auto& r) { return r.address == addr; });
    ASSERT_NE(it, devices.end());
    EXPECT_FALSE(it->is_faulty);
}

//gtest main

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

