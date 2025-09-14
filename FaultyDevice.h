#pragma once
#include <string>
#include <compare>
#include <cstdint>
#include <memory>     // ← добавлено для std::unique_ptr
#include "Device.h"

/**
 * @file FaultyDevice.h
 * @brief Объявление класса FaultyDevice и перечисления ServicePriority
 *
 */

 /**
  * @enum ServicePriority
  * @brief Приоритет обслуживания устройства
  */
enum class ServicePriority : uint8_t { None = 0, Low = 1, High = 2 };

/**
 * @class FaultyDevice
 * @brief Модель неисправного устройства
 *
 * хранит имя, IP-адрес, приоритет обслуживания
 * и текстовое описание неисправности. Поддерживает
 * оператор сравнения для сортировки
 */
class FaultyDevice {
public:
    /**
     * @brief Конструктор.
     * @param name   имя устройства
     * @param address IPv4-адрес
     * @param prio  Приоритет обслуживания
     * @param fault  описание неисправности
     */
    FaultyDevice(std::string name,
        uint32_t address,
        ServicePriority prio,
        std::string fault)
        : name_(std::move(name)),
        address_(address),
        priority_(prio),
        fault_(std::move(fault)) {
    }

    /// @name Геттеры
    ///@{
    /**
     * @brief Имя
     * @return Ссылка
     */
    const std::string& name() const noexcept { return name_; }

    /**
     * @brief IP-адрес
     * @return 32битное представление адреса
     */
    uint32_t address() const noexcept { return address_; }

    /**
     * @brief Текущий приоритет
     * @return Значение @ref ServicePriority.
     */
    ServicePriority priority() const noexcept { return priority_; }

    /**
     * @brief Описание неисправности
     * @return Ссылка на строку с описанием
     */
    const std::string& fault_description() const noexcept { return fault_; }
    ///@}

    /// @name Сеттеры
    ///@{
    /**
     * @brief Установить приоритет
     * @param newPrio Новое значение приоритета
     */
    void setPriority(ServicePriority newPrio) noexcept { priority_ = newPrio; }

    /**
     * @brief Обновить описание неисправности
     * @param newFault Новый текст
     */
    void setFault(std::string newFault) { fault_ = std::move(newFault); }
    ///@}

    /**
     * @brief Сравнение устройств для сортировки
     *
     *
     * @param rhs другое устройство
     * @return std::strong_ordering
     */
    std::strong_ordering operator<=>(const FaultyDevice& rhs) const noexcept {
        int lhsRank = rank(priority_);
        int rhsRank = rank(rhs.priority_);
        if (lhsRank != rhsRank) return lhsRank <=> rhsRank;

        if (name_ != rhs.name_) return name_ <=> rhs.name_;
        return address_ <=> rhs.address_;
    }

    /**
     * @brief Проверка равенства по приоритету.
     */
    bool operator==(const FaultyDevice& rhs) const noexcept {
        return priority_ == rhs.priority_;
    }

    /// @name IPv4 утилиты
    ///@{
    /**
     * @brief Преобразование IPv4-строки
     * @param dotted Строка IPv4.
     * @return 32битное представление адреса
     * @throws std::invalid_argument Если строка  имеет неверный формат
     * @throws std::out_of_range     Если какой-то октет вне диапазона
     */
    static uint32_t ipv4_to_u32(const std::string& dotted);

    /**
     * @brief Преобразование u32 IPv4 в строку
     * @param value 32битное представление IPv4
     */
    static std::string u32_to_ipv4(uint32_t value);
    ///@}

    //
    std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const;

private:
    /**
     * @brief Числовой ранг приоритета
     * @param p Приоритет
     * @return 1 для None, 2 для Low, 3 для High
     */
    static constexpr int rank(ServicePriority p) noexcept {
        switch (p) {
        case ServicePriority::High: return 3;
        case ServicePriority::Low:  return 2;
        case ServicePriority::None: return 1;
        }
        return 0;
    }


    std::string     name_;     ///< Имя
    uint32_t        address_;  ///< IP-адрес как 32битное число
    ServicePriority priority_; ///< Приоритет
    std::string     fault_;    ///< Текст неисправности.
};
