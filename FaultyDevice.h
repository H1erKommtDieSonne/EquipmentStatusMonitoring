#pragma once    //чтобы подключался 1 раз
#include <string>   //для строк
#include <compare>  //оператор <=>(универсальный оператор сравнения_) только на с++20
#include <cstdint>  //для уинтов
#include <stdexcept>    //для исключений
#include <sstream>  //для потоков

///приоритет обслуживания: High > Low > None
enum class ServicePriority : uint8_t { None = 0, Low = 1, High = 2 };   //типобезопасное перечисление

class FaultyDevice { //обьявляю класс
public:     //для ясности не приват
    FaultyDevice(std::string name,
        uint32_t address,
        ServicePriority prio,
        std::string fault)
        : name_(std::move(name)),   //забираем содержимое без копии(ОпТиМиЗаЦиЯ)
        address_(address),  //сетевой адрес 32 бита
        priority_(prio),    //ПРИОРИТЕТ
        fault_(std::move(fault)) {  //описание неисправности
    }

    //геттеры
    const std::string& name() const noexcept { return name_; }  //не изхменяем обьект и не бросаем исключений
    uint32_t address() const noexcept { return address_; }
    ServicePriority priority() const noexcept { return priority_; }
    const std::string& fault_description() const noexcept { return fault_; }

    //сеттеры
    void setPriority(ServicePriority newPrio) noexcept { priority_ = newPrio; } //меняем только приоритет
    void setFault(std::string newFault) { fault_ = std::move(newFault); }   //обновляем текст неисправности

    //сравнение по приоритету
    //чем выше приоритет тем больше объект
    std::strong_ordering operator<=>(const FaultyDevice& rhs) const noexcept {
        int lhsRank = rank(priority_);  //приоритет в число
        int rhsRank = rank(rhs.priority_);
        if (lhsRank != rhsRank) return lhsRank <=> rhsRank;

        //при равных приоритетах
        if (name_ != rhs.name_) return name_ <=> rhs.name_;
        return address_ <=> rhs.address_;
    }

    //равенство по приоритету
    bool operator==(const FaultyDevice& rhs) const noexcept {
        return priority_ == rhs.priority_;
    }

    //утилиты для IPv4
    // статические методы 
    //преобразование из a.b.c.d в 32битное число
    static uint32_t ipv4_to_u32(const std::string& dotted);
    static std::string u32_to_ipv4(uint32_t value);

private:    //добавляю вес каждому приоритету
    static constexpr int rank(ServicePriority p) noexcept {
        switch (p) {
        case ServicePriority::High: return 3;
        case ServicePriority::Low:  return 2;
        case ServicePriority::None: return 1;
        }
        return 0;
    }
    //поля
    std::string name_;
    uint32_t    address_;
    ServicePriority priority_;
    std::string fault_;
};
