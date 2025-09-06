#pragma once    //чтобы подключалс€ 1 раз
#include <string>   //дл€ строк
#include <compare>  //оператор <=>(универсальный оператор сравнени€_) только на с++20
#include <cstdint>  //дл€ уинтов
#include <stdexcept>    //дл€ исключений
#include <sstream>  //дл€ потоков

///приоритет обслуживани€: High > Low > None
enum class ServicePriority : uint8_t { None = 0, Low = 1, High = 2 };   //типобезопасное перечисление

class FaultyDevice { //обь€вл€ю класс
public:     //дл€ €сности не приват
    FaultyDevice(std::string name,
        uint32_t address,
        ServicePriority prio,
        std::string fault)
        : name_(std::move(name)),   //забираем содержимое без копии(ќп“ићи«а÷ия)
        address_(address),  //сетевой адрес 32 бита
        priority_(prio),    //ѕ–»ќ–»“≈“
        fault_(std::move(fault)) {  //описание неисправности
    }

    //геттеры
    const std::string& name() const noexcept { return name_; }  //не изхмен€ем обьект и не бросаем исключений
    uint32_t address() const noexcept { return address_; }
    ServicePriority priority() const noexcept { return priority_; }
    const std::string& fault_description() const noexcept { return fault_; }

    //сеттеры
    void setPriority(ServicePriority newPrio) noexcept { priority_ = newPrio; } //мен€ем только приоритет
    void setFault(std::string newFault) { fault_ = std::move(newFault); }   //обновл€ем текст неисправности

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

    //утилиты дл€ IPv4
    // статические методы 
    //преобразование из a.b.c.d в 32битное число
    static uint32_t ipv4_to_u32(const std::string& dotted);
    static std::string u32_to_ipv4(uint32_t value);

private:    //добавл€ю вес каждому приоритету
    static constexpr int rank(ServicePriority p) noexcept {
        switch (p) {
        case ServicePriority::High: return 3;
        case ServicePriority::Low:  return 2;
        case ServicePriority::None: return 1;
        }
        return 0;
    }
    //пол€
    std::string name_;
    uint32_t    address_;
    ServicePriority priority_;
    std::string fault_;
};
