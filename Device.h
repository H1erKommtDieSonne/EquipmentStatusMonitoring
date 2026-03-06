#pragma once
/**
 * @file Device.h
 * @brief Абстрактная база для всех устройств
 */

#include <cstdint>
#include <memory>
#include <string>
#include <sstream>

enum class ServicePriority : uint8_t;

/**
* @class Device
* @brief Абстрактная база для всех устройств
* Содержит общие поля и чисто виртуальные методы для
* определения состояния и переходов состояний в производных классах
*/
class Device {
public:
    
    using Address = uint32_t;

    /**
     * @brief Конструктор базового устройства
     * @param name Имя устройства
     * @param addr IPадрес устройства
     * @param prio Приоритет обслуживания
     */
    Device(std::string name, Address addr, ServicePriority prio)
        : name_(std::move(name)), address_(addr), priority_(prio) {
    }

    /// @brief Виртуальный деструктор
    virtual ~Device() = default;

    /**
     * @brief Полиморфное копирование
     * @return Указатель на копию конкретного объекта-потомка.
     */
    virtual std::unique_ptr<Device> clone() const = 0;

    /// @name Доступ к свойствам
    ///@{
    /// @brief Имя устройства
    const std::string& name() const noexcept { return name_; }
    /// @brief Адрес
    Address            address() const noexcept { return address_; }
    /// @brief Текущий приоритет
    ServicePriority    priority() const noexcept { return priority_; }
    ///@}

    /**
     * @brief Установить приоритет обслуживания
     */
    virtual void setPriority(ServicePriority p) { priority_ = p; }

    /**
     * @brief Признак резервного устройства
     * @details По умолчанию не резерв
     */
    virtual bool isReserve() const noexcept { return false; }

    /// @brief Признак неисправности устройства
    virtual bool isFaulty() const noexcept = 0;

    /// @brief Перевести устройство в состояние неисправно
    virtual std::unique_ptr<Device> breakDown(std::string fault) const = 0;

    /// @brief Ремонт устройства (создать новое исправное)
    virtual std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const = 0;

    
    virtual std::string toString() const;

    /**
     * @brief Требуется ли обслуживание
     * @return true если приоритет не равен нулю
     */
    bool requiresService() const noexcept { return priority_ != static_cast<ServicePriority>(0); }

protected:
    std::string     name_;
    Address         address_;
    ServicePriority priority_;
};
