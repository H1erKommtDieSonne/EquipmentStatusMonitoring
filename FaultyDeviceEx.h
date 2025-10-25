#pragma once
#include "Device.h"
#include <string>
#include <sstream>
#include <memory>
#include "ServicePriority.h"

/**
 * @class FaultyDeviceEx
 * @brief Неисправное устройство.
 */
class FaultyDeviceEx : public Device {
public:
    FaultyDeviceEx(std::string name, Address addr, ServicePriority prio, std::string fault)
        : Device(std::move(name),
            addr,
            (prio == static_cast<ServicePriority>(0) ? static_cast<ServicePriority>(1) : prio)),
        fault_(std::move(fault)) {
    }

    /// @brief Полиморфное копирование.
    std::unique_ptr<Device> clone() const override {
        return std::unique_ptr<Device>(new FaultyDeviceEx(*this));
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

    /// @brief Описание неисправности.
    const std::string& fault_description() const noexcept { return fault_; }

private:
    std::string fault_;
};
