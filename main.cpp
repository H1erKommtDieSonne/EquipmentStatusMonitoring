#include <cassert>  //макрос ассерт на проверку условия
#include <vector>   //контейцнер для динамич массивов
#include <algorithm>    //сортировка
#include <iostream> //ввод в консоль через коут
#include "FaultyDevice.h"

int main() {
    //создание
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.5");  //вызов сттатич метода
    FaultyDevice d1("Насос#1", addr, ServicePriority::High, "Перегрев");
    FaultyDevice d2("Клапан#2", FaultyDevice::ipv4_to_u32("10.0.0.6"),
        ServicePriority::Low, "Протечка");

    //геттеры и преобразование адреса
    assert(FaultyDevice::u32_to_ipv4(d1.address()) == "10.0.0.5");

    //изменение приоритета
    d2.setPriority(ServicePriority::High);
    assert(d2.priority() == ServicePriority::High);

    //сравнение по приоритету
    FaultyDevice a("A", addr, ServicePriority::High, "X");
    FaultyDevice b("B", addr, ServicePriority::Low, "Y");
    FaultyDevice c("C", addr, ServicePriority::None, "Z");

    assert((a <=> b) == std::strong_ordering::greater);
    assert((b <=> c) == std::strong_ordering::greater);
    assert((a == d2));

    //сортировка заявок по важности
    std::vector<FaultyDevice> v{ b, c, a };
    std::sort(v.begin(), v.end(), std::greater<>()); //убывание важности
    assert(v.front().priority() == ServicePriority::High);
    assert(v.back().priority() == ServicePriority::None);

    std::cout << "ALL TESTS PASSED\n"; //если мы дошли до сюда то тесты пройдены
    return 0;
}
