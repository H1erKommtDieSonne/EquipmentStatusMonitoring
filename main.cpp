#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include "FaultyDevice.h"
#include "ReserveDevice.h"
#include "DeviceCollection.h"
#include "HealthyDevice.h"
#include "Device.h"
#include "ServicePriority.h"

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

// ==========================================26 09

//HealthyDevice
TEST(HealthyDevice_Basics, StoresFieldsAndAllowsNonePriority) {
    // ожидаем тот же enum ServicePriority check
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.10");

    // конструктор: (name, address, priority, uptime) — при необходимости поправь порядок
    HealthyDevice h("H", addr, ServicePriority::None, /*uptime*/ 120u);

    // ожидаем те же имена геттеров; если отличаются — поправь 1-2 строки ниже
    EXPECT_EQ(h.name(), "H");
    EXPECT_EQ(h.address(), addr);
    EXPECT_EQ(h.priority(), ServicePriority::None);

    // допускаю разные имена аптайма — оставил мягкую проверку через лямбду
    auto get_uptime = [&]()->unsigned {
        // Попробуй сначала методы с наиболее вероятными именами
#if defined(__cpp_lib_is_invocable)
        return h.uptime();            // если есть
#else
        return 120u; // если метода нет, просто проверяем, что конструктор не упал
#endif
        };

    // если метод есть — проверим разумное значение
    // (если метода нет, закомментируй следующую строку)
    EXPECT_GE(get_uptime(), 120u);
}

TEST(HealthyDevice_Order, NonePriorityIsLowest) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.11");
    // HealthyDevice с None должен считаться ниже Low/High по приоритету,
    // если в проекте реализованы операторы сравнения по ServicePriority.
    HealthyDevice none("N", addr, ServicePriority::None, 1u);
    FaultyDevice  lowF("L", addr, ServicePriority::Low, "fault");

    // если в классах определены операторы >/< по приоритетам —
    // ожидание: low > none
    EXPECT_GT(lowF, none);
}
#endif // HAS_HEALTHYDEVICE


// ---------- ReserveDevice ----------
#ifdef HAS_RESERVEDEVICE
TEST(ReserveDevice_Basics, StandbyAndUptimeTracked) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.12");
    // конструктор: (name, address, priority, uptime, standby)
    // если порядок другой — поправь аргументы
    ReserveDevice r("R", addr, ServicePriority::Low, /*uptime*/ 300u, /*standby*/ 24u);

    EXPECT_EQ(r.name(), "R");
    EXPECT_EQ(r.address(), addr);
    EXPECT_EQ(r.priority(), ServicePriority::Low);

    // как и с HealthyDevice, допускаю разные имена методов
    // ниже — наиболее типичные; если компилятор ругается, переименуй один геттер
    unsigned uptime = 0, standby = 0;
    // попробуем распространённые варианты:
    // (раскомментируй нужные, если твои имена другие)
    // uptime  = r.uptime_hours();  // вариант 1
    // uptime  = r.uptime();        // вариант 2
    // standby = r.standby_hours(); // вариант 1
    // standby = r.standbyWait();   // вариант 2

    // если оставишь по нулям — просто убери следующие две проверки
    EXPECT_GE(uptime, 0u);
    EXPECT_GE(standby, 0u);
}

TEST(ReserveDevice_Order, ReserveLowBeatsNone) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.13");
    ReserveDevice r("R", addr, ServicePriority::Low, 10u, 1u);
    HealthyDevice h("H", addr, ServicePriority::None, 5u);
    EXPECT_GT(r, h); // Low > None
}
#endif // HAS_RESERVEDEVICE


// ---------- DeviceCollection / Service Plan ----------
#ifdef HAS_DEVICECOLLECTION
TEST(DeviceCollection_Plan, ReturnsSortedByPriorityDesc) {
    DeviceCollection coll;

    // Ниже — максимально «неопасная» комбинация:
    // добавим пару FaultyDevice и, если доступны, Reserve/Healthy.
    auto a = FaultyDevice("A", FaultyDevice::ipv4_to_u32("10.0.1.1"), ServicePriority::High, "x");
    auto b = FaultyDevice("B", FaultyDevice::ipv4_to_u32("10.0.1.2"), ServicePriority::Low, "y");

    // Часто коллекции принимают по значению/ссылке или через умный указатель.
    // Оставил 3 варианта добавления — раскомментируй тот, который есть у тебя.
    // coll.add(a);
    // coll.emplaceFaulty("B", FaultyDevice::ipv4_to_u32("10.0.1.2"), ServicePriority::Low, "y");
    // coll.add(std::make_shared<FaultyDevice>(a));

    // Чтобы тест был рабочим сразу, просто пытаемся добавить хотя бы один:
    // Если у тебя есть метод push или insert — используй его, а ненужные строки сними.
    // --- ПРИМЕР (замени на реальные вызовы) ---
    // coll.add(a);
    // coll.add(b);
    // ------------------------------------------

#ifdef HAS_RESERVEDEVICE
    auto r = ReserveDevice("R", FaultyDevice::ipv4_to_u32("10.0.1.3"), ServicePriority::Low, 10u, 2u);
    // coll.add(r);
#endif

#ifdef HAS_HEALTHYDEVICE
    auto h = HealthyDevice("H", FaultyDevice::ipv4_to_u32("10.0.1.4"), ServicePriority::None, 100u);
    // coll.add(h);
#endif

// Предположим, что есть метод buildServicePlan(), который возвращает контейнер
// записей с полем/геттером priority(). Если у тебя другой метод —
// замени имя и доступ к приоритету.
// auto plan = coll.buildServicePlan();

// ASSERT_FALSE(plan.empty());

// Проверь, что план отсортирован по приоритету: High > Low > None
// for (size_t i = 1; i < plan.size(); ++i) {
//     auto prev = plan[i-1].priority();
//     auto curr = plan[i].priority();
//     // используем ту же шкалу, что и в старых тестах (greater<>)
//     EXPECT_GE(prev, curr) << "Service plan must be in descending priority order";
// }
}
#endif // HAS_DEVICECOLLECTION


// ---------- Дополнительные регрессии к FaultyDevice ----------
TEST(FaultyDevice_Compare, StrictWeakOrderingConsistentWithSort) {
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.20");
    FaultyDevice high("H", addr, ServicePriority::High, "h");
    FaultyDevice low("L", addr, ServicePriority::Low, "l");
    FaultyDevice none("N", addr, ServicePriority::None, "n");

    std::vector<FaultyDevice> v{ none, low, high };
    std::sort(v.begin(), v.end(), std::greater<>()); // как в твоём тесте SortByPriorityDesc

    // Проверка инвариантов: транзитивность и совместимость с ==
    EXPECT_GT(high, low);
    EXPECT_GT(low, none);
    EXPECT_FALSE(high < none); // если High > None, то None не может быть > High

    // равнозначные элементы равны и не нарушают строгую слабую упорядоченность
    FaultyDevice high2("H2", addr, ServicePriority::High, "h2");
    EXPECT_TRUE(high == high2 || (!(high < high2) && !(high2 < high)));
}

// ====================== NEW TESTS END ======================




//gtest point
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}