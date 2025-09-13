#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include "FaultyDevice.h"

//easy test
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

//negative tests
TEST(FaultyDevice_Ip, InvalidFormat) {
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0"), std::invalid_argument); //lose octet
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0.1.2"), std::invalid_argument); //+octet
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10-0-0-5"), std::invalid_argument); //divine
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10,0,0,5"), std::invalid_argument);//non dots
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("a.b.c.d"), std::invalid_argument); //non numbers
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

//priority sort
TEST(FaultyDevice_Order, PriorityAndTies) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.5");
    FaultyDevice a("A", addr, ServicePriority::High, "X");
    FaultyDevice b("B", addr, ServicePriority::Low, "Y");
    FaultyDevice c("C", addr, ServicePriority::None, "Z");

    EXPECT_GT(a, b); //high > low
    EXPECT_GT(b, c); //low  > none

    FaultyDevice d("D", addr, ServicePriority::High, "Q");
    EXPECT_TRUE(a == d); //eq
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

//memory test
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
        EXPECT_TRUE(inserted); //unique
    }
    EXPECT_EQ(seen.size(), devices.size());
}

//setter
TEST(FaultyDevice_Mutators, PriorityAndFault) {
    FaultyDevice x("X", FaultyDevice::ipv4_to_u32("1.2.3.4"), ServicePriority::None, "ok");
    x.setPriority(ServicePriority::High);
    EXPECT_EQ(x.priority(), ServicePriority::High);

    EXPECT_NO_THROW(x.setFault("new description"));
    EXPECT_EQ(x.fault_description(), "new description");
}

//gtest point
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}





//
///
//
//
//
//
//
///
///
/// /
/// /
/// /
/// /
/// 
/// //
/// /
/// /
/// /
/// /
/// 
/// 
///
/// //
#include <cassert>
#include <vector>
#include <algorithm>
#include "FaultyDevice.h"
#include <iostream>
#include <unordered_set>
#include <string>


//easy test
// тут нужно было сделать так pered mainom
//
//
    /////     /////       /////       /////      /////
static int tests_run = 0;
static int tests_failed = 0;

#define CHECK_TRUE(expr) do {                    \
    ++tests_run;                                  \
    if (!(expr)) {                                \
        ++tests_failed;                           \
        std::cerr << "[FAIL] " #expr << "\n";     \
    }                                             \
} while(0)

#define CHECK_EQ(a,b) do {                        \
    ++tests_run;                                  \
    if (!((a) == (b))) {                          \
        ++tests_failed;                           \
        std::cerr << "[FAIL] " #a " == " #b       \
                  << " (got: " << (a)             \
                  << ", expected: " << (b) << ")\n"; \
    }                                             \
} while(0)

// Проверка, что выражение выбросит исклбючени типа E
#define CHECK_THROW(expr, E) do {                 \
    ++tests_run;                                  \
    bool thrown = false;                          \
    try { (void)(expr); }                         \
    catch (const E&) { thrown = true; }           \
    catch (...) {}                                \
    if (!thrown) {                                \
        ++tests_failed;                           \
        std::cerr << "[FAIL] expected throw " #E  \
                  << " for " #expr << "\n";       \
    }                                             \
} while(0)

// Проверка, что выражение не бросит исключений
#define CHECK_NO_THROW(expr) do {                 \
    ++tests_run;                                  \
    try { (void)(expr); }                         \
    catch (...) {                                 \
        ++tests_failed;                           \
        std::cerr << "[FAIL] unexpected exception: " #expr << "\n"; \
    }                                             \
} while(0)
//     /////        /////         //////         /////        /////   /////
//
//
//
//
//
//

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK_TRUE(expr) do {                    
++tests_run;
if (!(expr)) {
    ++tests_failed;
    std::cerr << "[FAIL] " #expr << "\n";
}
} while (0)

#define CHECK_EQ(a,b) do {                        
++tests_run;
if (!((a) == (b))) {
    ++tests_failed;
    std::cerr << "[FAIL] " #a " == " #b
        << " (got: " << (a)
        << ", expected: " << (b) << ")\n";
}
} while (0)

//drop E exception
#define CHECK_THROW(expr, E) do {                 
++tests_run;
bool thrown = false;
try { (void)(expr); }
catch (const E&) { thrown = true; }
catch (...) {}
if (!thrown) {
    ++tests_failed;
    std::cerr << "[FAIL] expected throw " #E
        << " for " #expr << "\n";
}
} while (0)

//non drop exeption
#define CHECK_NO_THROW(expr) do {                 
++tests_run;
try { (void)(expr); }
catch (...) {
    ++tests_failed;
    std::cerr << "[FAIL] unexpected exeption: " #expr << "\n";
}
} while (0)

int main() {

    //positive tests

    {
        //1
        uint32_t a = FaultyDevice::ipv4_to_u32("10.0.0.5");
        CHECK_EQ(FaultyDevice::u32_to_ipv4(a), std::string("10.0.0.5"));

        //diapazon
        CHECK_EQ(FaultyDevice::u32_to_ipv4(FaultyDevice::ipv4_to_u32("0.0.0.0")), std::string("0.0.0.0"));
        CHECK_EQ(FaultyDevice::u32_to_ipv4(FaultyDevice::ipv4_to_u32("255.255.255.255")), std::string("255.255.255.255"));

        //typical id
        const char* samples[] = { "1.2.3.4", "192.168.0.1", "172.16.5.10", "8.8.8.8" };
        for (auto s : samples) {
            uint32_t x = FaultyDevice::ipv4_to_u32(s);
            CHECK_EQ(FaultyDevice::u32_to_ipv4(x), std::string(s));
        }
    }


    //negative tests

    {
        //non right format
        CHECK_THROW(FaultyDevice::ipv4_to_u32("10.0.0"), std::invalid_argument);
        CHECK_THROW(FaultyDevice::ipv4_to_u32("10.0.0.1.2"), std::invalid_argument);

        //non dots
        CHECK_THROW(FaultyDevice::ipv4_to_u32("10-0-0-5"), std::invalid_argument);
        CHECK_THROW(FaultyDevice::ipv4_to_u32("10,0,0,5"), std::invalid_argument);

        //non numbers
        CHECK_THROW(FaultyDevice::ipv4_to_u32("a.b.c.d"), std::invalid_argument);
        CHECK_THROW(FaultyDevice::ipv4_to_u32("10.a.0.5"), std::invalid_argument);

        //invalid string
        CHECK_THROW(FaultyDevice::ipv4_to_u32(""), std::invalid_argument);
        CHECK_THROW(FaultyDevice::ipv4_to_u32("   "), std::invalid_argument);

        //invalid type
        CHECK_THROW(FaultyDevice::ipv4_to_u32("10.0.0.5abc"), std::invalid_argument);

        //out of diapazon
        CHECK_THROW(FaultyDevice::ipv4_to_u32("256.0.0.1"), std::out_of_range);
        CHECK_THROW(FaultyDevice::ipv4_to_u32("10.300.0.1"), std::out_of_range);
        CHECK_THROW(FaultyDevice::ipv4_to_u32("10.0.-1.1"), std::invalid_argument);
    }


    //priority comparison

    {
        auto addr = FaultyDevice::ipv4_to_u32("10.0.0.5");
        FaultyDevice a("A", addr, ServicePriority::High, "X");
        FaultyDevice b("B", addr, ServicePriority::Low, "Y");
        FaultyDevice c("C", addr, ServicePriority::None, "Z");

        CHECK_TRUE((a <=> b) == std::strong_ordering::greater);
        CHECK_TRUE((b <=> c) == std::strong_ordering::greater);

        //eq
        FaultyDevice d("D", addr, ServicePriority::High, "Q");
        CHECK_TRUE(a == d);

        //sort
        std::vector<FaultyDevice> v{ b, c, a };
        std::sort(v.begin(), v.end(), std::greater<>());
        CHECK_TRUE(v.front().priority() == ServicePriority::High);
        CHECK_TRUE(v.back().priority() == ServicePriority::None);
    }


    //where adress

    //  (uint32_t FaultyDevice::address_).

    {
        std::vector<std::string> ips = {
            "10.0.0.5", "10.0.0.6", "192.168.0.1", "8.8.8.8"
        };
        std::vector<FaultyDevice> devices;
        devices.reserve(ips.size());

        //make units and put into conteiner
        for (size_t i = 0; i < ips.size(); ++i) {
            uint32_t addr = FaultyDevice::ipv4_to_u32(ips[i]);
            devices.emplace_back("Dev#" + std::to_string(i), addr, ServicePriority::Low, "test");
        }

        //check eq
        for (size_t i = 0; i < devices.size(); ++i) {
            std::string back = FaultyDevice::u32_to_ipv4(devices[i].address());
            CHECK_EQ(back, ips[i]);
        }

        //check unique
        std::unordered_set<uint32_t> seen;
        for (auto& d : devices) {
            CHECK_TRUE(seen.insert(d.address()).second);
        }
    }


    //setPriority / setFault

    {
        FaultyDevice x("X", FaultyDevice::ipv4_to_u32("1.2.3.4"), ServicePriority::None, "ok");
        CHECK_EQ(static_cast<int>(x.priority()), static_cast<int>(ServicePriority::None));
        x.setPriority(ServicePriority::High);
        CHECK_EQ(static_cast<int>(x.priority()), static_cast<int>(ServicePriority::High));

        CHECK_NO_THROW(x.setFault("new description"));
        CHECK_TRUE(x.fault_description() == std::string("new description"));
    }


    //final check

    if (tests_failed == 0) {
        std::cout << "[OK] All tests passed (" << tests_run << " checks).\n";
        return 0;
    }
    else {
        std::cerr << "[FAIL] " << tests_failed << " / " << tests_run << " checks failed.\n";
        return 1;
    }
}
