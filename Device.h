#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <sstream>

enum class ServicePriority : uint8_t;

/**
* @class Device
* @brief Абстрактная база для всех устройств
* Содержит общие поля (имя, адрес, приоритет) и чисто виртуальные методы для
* определения состояния и переходов состояний в производных классах
*/

class Device {
public:
    using Address = uint32_t;

    //
    /**
     * @brief Конструктор базового устройства
     * @param name Имя устройства
     * @param addr IPадрес устройства
     * @param prio Приоритет обслуживания
     */

    Device(std::string name, Address addr, ServicePriority prio)
        : name_(std::move(name)), address_(addr), priority_(prio) {
    }

    virtual ~Device() = default;

    /// @name Доступ к свойствам
    ///@{
    /**
     * @brief Имя устройства
     * @return Ссылка на строку с именем
     */

    const std::string& name() const noexcept { return name_; }
    Address            address() const noexcept { return address_; }
    ServicePriority    priority() const noexcept { return priority_; }

    ///@}

    
    /**
     * @brief Установить приоритет обслуживания
     * @param p Новое значение приоритета
     * @note Поведение и ограничения могут уточняться в производных классах
     */
    
    virtual void setPriority(ServicePriority p) { priority_ = p; }

    /**
     * @brief Признак того, что устройство неисправно
     * @return true, если устройство неисправно, иначе false
     */

    virtual bool isFaulty() const noexcept = 0;


    virtual std::unique_ptr<Device> breakDown(std::string fault) const = 0;


    virtual std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const = 0;

    
    virtual std::string toString() const;

    /**
     * @brief Требуется ли обслуживание
     * По умолчанию считает что обслуживание требуется для любого приоритета,
     * отличного от нулевого
     * @return true, если приоритет не равен нулю
     */

    bool requiresService() const noexcept { return priority_ != static_cast<ServicePriority>(0); }

protected:
    std::string     name_;
    Address         address_;
    ServicePriority priority_;
};
