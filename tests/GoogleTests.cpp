#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include "FaultyDevice.h"

// --- Позитивные сценарии: round-trip и границы ---
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

// --- Негативные сценарии: формат/диапазон/хвост ---
TEST(FaultyDevice_Ip, InvalidFormat) {
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0"), std::invalid_argument); // мало октетов
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0.1.2"), std::invalid_argument); // много октетов
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10-0-0-5"), std::invalid_argument); // разделители
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10,0,0,5"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("a.b.c.d"), std::invalid_argument); // не числа
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.a.0.5"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32(""), std::invalid_argument); // пустая
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("   "), std::invalid_argument); // пробелы
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0.5abc"), std::invalid_argument); // хвост
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.-1.1"), std::invalid_argument); // отрицат.
}

TEST(FaultyDevice_Ip, OutOfRangeOctet) {
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("256.0.0.1"), std::out_of_range);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.300.0.1"), std::out_of_range);
}

// --- Порядок/сортировка по приоритету ---
TEST(FaultyDevice_Order, PriorityAndTies) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.5");
    FaultyDevice a("A", addr, ServicePriority::High, "X");
    FaultyDevice b("B", addr, ServicePriority::Low, "Y");
    FaultyDevice c("C", addr, ServicePriority::None, "Z");

    EXPECT_GT(a, b); // High > Low
    EXPECT_GT(b, c); // Low  > None

    FaultyDevice d("D", addr, ServicePriority::High, "Q");
    EXPECT_TRUE(a == d); // равны по приоритету
}

TEST(FaultyDevice_Order, SortByPriorityDesc) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.5");
    FaultyDevice a("A", addr, ServicePriority::High, "X");
    FaultyDevice b("B", addr, ServicePriority::Low, "Y");
    FaultyDevice c("C", addr, ServicePriority::None, "Z");

    std::vector<FaultyDevice> v{ b, c, a };
    std::sort(v.begin(), v.end(), std::greater<>()); // по убыванию
    EXPECT_EQ(v.front().priority(), ServicePriority::High);
    EXPECT_EQ(v.back().priority(), ServicePriority::None);
}

// --- Куда «лежат» адреса: хранение и уникальность ---
TEST(FaultyDevice_Address, StoredAndUnique) {
    std::vector<std::string> ips = { "10.0.0.5", "10.0.0.6", "192.168.0.1", "8.8.8.8" };

    std::vector<FaultyDevice> devices;
    devices.reserve(ips.size());

    for (size_t i = 0; i < ips.size(); ++i) {
        uint32_t addr = FaultyDevice::ipv4_to_u32(ips[i]);
        devices.emplace_back("Dev#" + std::to_string(i), addr, ServicePriority::Low, "test");
    }

    for (size_t i = 0; i < devices.size(); ++i) {
        EXPECT_EQ(FaultyDevice::u32_to_ipv4(devices[i].address()), ips[i]); // round-trip
    }

    std::unordered_set<uint32_t> seen;
    for (auto& d : devices) {
        auto [it, inserted] = seen.insert(d.address());
        EXPECT_TRUE(inserted); // адрес уникален
    }
    EXPECT_EQ(seen.size(), devices.size());
}

// --- Сеттеры ---
TEST(FaultyDevice_Mutators, PriorityAndFault) {
    FaultyDevice x("X", FaultyDevice::ipv4_to_u32("1.2.3.4"), ServicePriority::None, "ok");
    x.setPriority(ServicePriority::High);
    EXPECT_EQ(x.priority(), ServicePriority::High);

    EXPECT_NO_THROW(x.setFault("new description"));
    EXPECT_EQ(x.fault_description(), "new description");
}

// Точка входа для gtest
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
