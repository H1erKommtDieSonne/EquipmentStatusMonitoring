#pragma once
#include <string>
#include <compare>
#include <cstdint>
#include <memory>
#include "Device.h"
#include "ServicePriority.h"

/**
 * @class FaultyDevice
 * @brief Модель неисправного устройства
 * Используется там, где исторически применялся отдельный тип. Для совместимости
 * предоставляет метод @ref repair, создающий объект из иерархии @ref Device
 */

class FaultyDevice {
public:
    /**
      * @brief Конструктор
      * @param name Имя устройства
      * @param address IPадрес
      * @param prio Приоритет обслуживания
      * @param fault описание неисправности
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
    * @brief Имя устройства
    * @return Ссылка на строку с именем
    */

    const std::string& name() const noexcept { return name_; }

    /**
     * @brief IPадрес устройства
     * @return 32битное представление IPадреса
     */

    uint32_t address() const noexcept { return address_; }

    /**
     * @brief Текущий приоритет обслуживания
     * @return Значение @ref ServicePriority
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
     * @brief Установить приоритет обслуживания
     * @param newPrio Новое значение приоритета
     */

    void setPriority(ServicePriority newPrio) noexcept { priority_ = newPrio; }

    /**
    * @brief Обновить описание неисправности
    * @param newFault Новый текст описания
    */

    void setFault(std::string newFault) { fault_ = std::move(newFault); }
    
    ///@}

    /**
     * @brief Сравнение для сортировки
     *
     * Сортирует по убыванию приоритета, затем по имени,
     * затем по адресу
     * @param rhs Другое устройство
     * @return Результат сравнения
     */

    std::strong_ordering operator<=>(const FaultyDevice& rhs) const noexcept {
        int lhsRank = rank(priority_);
        int rhsRank = rank(rhs.priority_);
        if (lhsRank != rhsRank) return lhsRank <=> rhsRank;

        if (name_ != rhs.name_) return name_ <=> rhs.name_;
        return address_ <=> rhs.address_;
    }

    /**
     * @brief Равенство по приоритету
     * @param rhs Другое устройство
     * @return true, если приоритеты равны
     */

    bool operator==(const FaultyDevice& rhs) const noexcept {
        return priority_ == rhs.priority_;
    }

    /// @name IPv4 утилиты
   ///@{
   /**
    * @brief Преобразование IPстроки в число
    * @param Строка IPv4
    * @return 32битное представление адреса
    */
    
    static uint32_t ipv4_to_u32(const std::string& dotted);

    /**
   * @brief Преобразование 32битного IPv4 в строковый вид
   * @param value 32битное представление IPv4
   * @return Строка в десятичном формате
   */

    static std::string u32_to_ipv4(uint32_t value);
    
    ///@}

   /**
    * @brief Фикс — перевод исторической модели в объект иерархии @ref Device
    * Создаёт и возвращает новый @ref HealthyDevice с тем же именем, адресом
    * и приоритетом, но с заданной наработкой после ремонта.
    * @param uptimeAfterRepairSec Наработка после ремонта
    * @return Указатель на новый объект из иерархии @ref Device
    */

    std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const;

private:
    
    /**
     * @brief Числовое значение приоритета для сортировки
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


    std::string     name_; ///< Имя устройства
    uint32_t        address_; ///< IPадрес как число
    ServicePriority priority_; ///< Приоритет обслуживания
    std::string     fault_; ///< Описание неисправности   
};
