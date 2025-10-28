#pragma once
#include "Device.h"
#include <memory>
#include <string>
#include <sstream>

/**
* @class HealthyDevice
* @brief Исправное устройство 
*/
class HealthyDevice : public Device {
public:
    /// @brief Конструктор
    HealthyDevice(std::string name, Address addr, ServicePriority prio, uint64_t uptimeSec)
        : Device(std::move(name), addr, prio), uptimeSec_(uptimeSec) {
    }

    /// @brief Полиморфное копирование
    std::unique_ptr<Device> clone() const override {
        return std::unique_ptr<Device>(new HealthyDevice(*this));
    }

    
    bool isFaulty() const noexcept override { return false; }

    /// @brief Наработка
    uint64_t uptime() const noexcept { return uptimeSec_; }

    std::unique_ptr<Device> breakDown(std::string fault) const override;
    std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const override;

    /// @brief Установить приоритет
    void setPriority(ServicePriority p) override { priority_ = p; }

    /// @brief Строка для логов
    std::string toString() const override {
        std::ostringstream os;
        os << "HealthyDevice{name=" << name_
            << ", addr=" << address_
            << ", prio=" << static_cast<int>(priority_)
            << ", uptimeSec=" << uptimeSec_ << "}";
        return os.str();
    }

protected:
    uint64_t uptimeSec_{ 0 };
};
