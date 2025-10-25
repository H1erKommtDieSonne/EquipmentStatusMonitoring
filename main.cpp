/**
 * @file tests_devices.cpp
 * @brief Набор unit-тестов для моделей устройств и вспомогательных функций.
 * @details
 *  Покрывает:
 *   - преобразование IPv4 <-> u32;
 *   - сравнение и сортировку по приоритетам обслуживания;
 *   - базовые инварианты для Healthy/Reserve устройств (если доступны);
 *   - заготовку под проверку DeviceCollection (если доступна).
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>   // std::sort
#include <unordered_set>
#include <functional>  // std::greater
#include <cstdint>     // uint32_t
#include <stdexcept>   // std::invalid_argument, std::out_of_range

#include "FaultyDevice.h"
#include "ReserveDevice.h"
#include "DeviceCollection.h"
#include "HealthyDevice.h"
#include "Device.h"
#include "ServicePriority.h"

 // ============================= FaultyDevice: IPv4 =============================

 /// @test Круговой прогон IPv4: строка -> u32 -> строка, базовые случаи.
TEST(FaultyDevice_Ip, RoundTripBasic) {
    uint32_t v = FaultyDevice::ipv4_to_u32("10.0.0.5");
    EXPECT_EQ(FaultyDevice::u32_to_ipv4(v), "10.0.0.5");

    v = FaultyDevice::ipv4_to_u32("192.168.0.1");
    EXPECT_EQ(FaultyDevice::u32_to_ipv4(v), "192.168.0.1");
}

/// @test Круговой прогон IPv4: граничные адреса.
TEST(FaultyDevice_Ip, RoundTripEdges) {
    EXPECT_EQ(FaultyDevice::u32_to_ipv4(FaultyDevice::ipv4_to_u32("0.0.0.0")), "0.0.0.0");
    EXPECT_EQ(FaultyDevice::u32_to_ipv4(FaultyDevice::ipv4_to_u32("255.255.255.255")), "255.255.255.255");
}

/// @test Круговой прогон IPv4: несколько произвольных образцов.
TEST(FaultyDevice_Ip, RoundTripSamples) {
    const char* samples[] = { "1.2.3.4", "172.16.5.10", "8.8.8.8" };
    for (auto s : samples) {
        uint32_t x = FaultyDevice::ipv4_to_u32(s);
        EXPECT_EQ(FaultyDevice::u32_to_ipv4(x), std::string(s));
    }
}

/// @test Негативные сценарии: формат строки некорректен.
TEST(FaultyDevice_Ip, InvalidFormat) {
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0"), std::invalid_argument);     // отсутствует октет
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0.1.2"), std::invalid_argument); // лишний октет
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10-0-0-5"), std::invalid_argument);   // неверный разделитель
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10,0,0,5"), std::invalid_argument);   // не точки
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("a.b.c.d"), std::invalid_argument);    // не числа
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.a.0.5"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32(""), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("   "), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.0.5abc"), std::invalid_argument);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.0.-1.1"), std::invalid_argument);
}

/// @test Негативные сценарии: октет выходит за допустимый диапазон.
TEST(FaultyDevice_Ip, OutOfRangeOctet) {
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("256.0.0.1"), std::out_of_range);
    EXPECT_THROW(FaultyDevice::ipv4_to_u32("10.300.0.1"), std::out_of_range);
}

// ============================= FaultyDevice: порядок ==========================

/// @test Сравнение по приоритетам и равенство устройств с одинаковым приоритетом.
TEST(FaultyDevice_Order, PriorityAndTies) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.5");
    FaultyDevice a("A", addr, ServicePriority::High, "X");
    FaultyDevice b("B", addr, ServicePriority::Low, "Y");
    FaultyDevice c("C", addr, ServicePriority::None, "Z");

    EXPECT_GT(a, b); // High > Low
    EXPECT_GT(b, c); // Low  > None

    FaultyDevice d("D", addr, ServicePriority::High, "Q");
    EXPECT_TRUE(a == d); // равны по приоритету и (по задумке) по порядку
}

/// @test Сортировка по убыванию приоритета.
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

// ============================ FaultyDevice: память/адрес ======================

/// @test Адрес хранится без искажений, адреса уникальны.
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
        EXPECT_TRUE(inserted); // адрес ещё не встречался
    }
    EXPECT_EQ(seen.size(), devices.size());
}

// ============================== HealthyDevice =================================

#ifdef HAS_HEALTHYDEVICE

/// @test HealthyDevice хранит поля и допускает None-приоритет.
TEST(HealthyDevice_Basics, StoresFieldsAndAllowsNonePriority) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.10");
    HealthyDevice h("H", addr, ServicePriority::None, 120u);

    EXPECT_EQ(h.name(), "H");
    EXPECT_EQ(h.address(), addr);
    EXPECT_EQ(h.priority(), ServicePriority::None);

    /// \todo Когда добавится геттер аптайма (например, uptime() или uptime_hours()),
    ///       добавить проверку значения 120u.
}

/// @test None-приоритет у HealthyDevice — наименьший.
TEST(HealthyDevice_Order, NonePriorityIsLowest) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.11");
    HealthyDevice none("N", addr, ServicePriority::None, 1u);
    FaultyDevice  lowF("L", addr, ServicePriority::Low, "fault");

    // Сравниваем именно приоритеты, а не разные типы устройств.
    EXPECT_GT(static_cast<int>(lowF.priority()), static_cast<int>(none.priority()));
}
#endif // HAS_HEALTHYDEVICE

// ============================== ReserveDevice =================================

#ifdef HAS_RESERVEDEVICE
/// @test Базовые поля ReserveDevice (standby/uptime можно расширить при наличии геттеров).
TEST(ReserveDevice_Basics, StandbyAndUptimeTracked) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.12");
    // Порядок аргументов сверяй с реальным конструктором:
    ReserveDevice r("R", addr, ServicePriority::Low, /*uptime*/ 300u, /*standby*/ 24u);

    EXPECT_EQ(r.name(), "R");
    EXPECT_EQ(r.address(), addr);
    EXPECT_EQ(r.priority(), ServicePriority::Low);

    /// \todo Если есть геттеры uptime()/uptime_hours() и standby()/standby_hours(),
    ///       добавить проверки на корректные значения (300u и 24u).
}

/// @test Приоритет Low (reserve) выше, чем None (healthy).
TEST(ReserveDevice_Order, ReserveLowBeatsNone) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.13");
    ReserveDevice r("R", addr, ServicePriority::Low, 10u, 1u);
    HealthyDevice h("H", addr, ServicePriority::None, 5u);
    EXPECT_GT(static_cast<int>(r.priority()), static_cast<int>(h.priority())); // Low > None
}
#endif // HAS_RESERVEDEVICE

// ============================ DeviceCollection / План =========================

#ifdef HAS_DEVICECOLLECTION
/// @test (DISABLED) План обслуживания отсортирован по убыванию приоритетов.
/// @note Тест отключён до появления стабильного API у DeviceCollection.
TEST(DISABLED_DeviceCollection_Plan, ReturnsSortedByPriorityDesc) {
    DeviceCollection coll;

    auto a = FaultyDevice("A", FaultyDevice::ipv4_to_u32("10.0.1.1"), ServicePriority::High, "x");
    auto b = FaultyDevice("B", FaultyDevice::ipv4_to_u32("10.0.1.2"), ServicePriority::Low, "y");

    // \todo Раскомментировать и адаптировать под доступный интерфейс коллекции.
    // coll.add(a);
    // coll.add(b);

    // auto plan = coll.buildServicePlan();
    // ASSERT_FALSE(plan.empty());
    // for (size_t i = 1; i < plan.size(); ++i) {
    //     EXPECT_GE(plan[i-1].priority(), plan[i].priority())
    //         << "Service plan must be in descending priority order";
    // }
}
#endif // HAS_DEVICECOLLECTION

// ======================== Доп. инварианты сравнения ===========================

/// @test Строгая слабая упорядоченность согласована с сортировкой и ==
TEST(FaultyDevice_Compare, StrictWeakOrderingConsistentWithSort) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.20");
    FaultyDevice high("H", addr, ServicePriority::High, "h");
    FaultyDevice low("L", addr, ServicePriority::Low, "l");
    FaultyDevice none("N", addr, ServicePriority::None, "n");

    std::vector<FaultyDevice> v{ none, low, high };
    std::sort(v.begin(), v.end(), std::greater<>()); // как в тесте SortByPriorityDesc

    EXPECT_GT(high, low);
    EXPECT_GT(low, none);
    EXPECT_FALSE(high < none);

    FaultyDevice high2("H2", addr, ServicePriority::High, "h2");
    EXPECT_TRUE(high == high2 || (!(high < high2) && !(high2 < high)));
}

//

/**
 * @brief Точка входа для запуска тестов Google Test.
 * @param argc количество аргументов командной строки
 * @param argv аргументы командной строки
 * @return код возврата RUN_ALL_TESTS()
 */
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
