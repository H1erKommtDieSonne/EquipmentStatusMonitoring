#include "FaultyDeviceEx.h"
#include "HealthyDevice.h"
#include <memory>

std::unique_ptr<Device> FaultyDeviceEx::breakDown(std::string) const {
    
    return std::unique_ptr<Device>(new FaultyDeviceEx(*this));
}

std::unique_ptr<Device> FaultyDeviceEx::repair(uint64_t uptimeAfterRepairSec) const {
    return std::unique_ptr<Device>(new HealthyDevice(name_, address_, priority_, uptimeAfterRepairSec));
}
