#pragma once
#include "Device.h"
#include <string>
#include <sstream>
#include <memory>
#include "ServicePriority.h"

/**
 * @class FaultyDeviceEx
 * @brief Модель неисправного устройства
 *
 * Хранит описание неисправности и обеспечивает
 *  - корректную инициализацию приоритета
 *  - признак неисправности
 *  - переходы состояний @ref breakDown и @ref repair
 */

class FaultyDeviceEx : public Device {
public:

    /**
 * @brief Конструктор неисправного устройства
 * Если в качестве приоритета приходит значение 0
 * оно автоматически повы1шается до 1
 * @param name Имя устройства
 * @param addr IP адрес устройства
 * @param prio Приоритет обслуживания
 * @param fault Текстовое описание неисправности
 */

    FaultyDeviceEx(std::string name, Address addr, ServicePriority prio, std::string fault)
        : Device(std::move(name),
            addr,
            
            (prio == static_cast<ServicePriority>(0) ? static_cast<ServicePriority>(1) : prio)),
        fault_(std::move(fault)) {
    }

    bool isFaulty() const noexcept override { return true; }

    
    std::unique_ptr<Device> breakDown(std::string) const override;
    std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const override;

    std::string toString() const override {
        std::ostringstream os;
        os << "FaultyDeviceEx{name=" << name_
            << ", addr=" << address_
            << ", prio=" << static_cast<int>(priority_)
            << ", fault=\"" << fault_ << "\"}";
        return os.str();
    }

    const std::string& fault_description() const noexcept { return fault_; }

private:
    std::string fault_;
};
