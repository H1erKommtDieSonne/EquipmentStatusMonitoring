#include "HealthyDevice.h"
#include "FaultyDeviceEx.h"

#include <memory>
#include <stdexcept>
#include <utility>

std::unique_ptr<Device> HealthyDevice::breakDown(std::string fault) const {
    if (fault.empty()) {
        throw std::invalid_argument("breakDown: empty fault description");
    }

    
    auto nextPrio = priority_;
    if (priority_ == static_cast<ServicePriority>(0)) {
        nextPrio = static_cast<ServicePriority>(1);
    }

    return std::unique_ptr<Device>(new FaultyDeviceEx(
        name_, address_, nextPrio, std::move(fault)
    ));
}

std::unique_ptr<Device> HealthyDevice::repair(uint64_t uptimeAfterRepairSec) const {
    return std::unique_ptr<Device>(new HealthyDevice(
        name_, address_, priority_, uptimeAfterRepairSec
    ));
}
