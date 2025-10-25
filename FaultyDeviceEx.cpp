#include "FaultyDeviceEx.h"
#include "HealthyDevice.h"
#include <memory>
#include <utility>

/**
 * @brief Повторная поломка: копия самого себя.
 */
std::unique_ptr<Device> FaultyDeviceEx::breakDown(std::string) const {
    return std::unique_ptr<Device>(new FaultyDeviceEx(*this));
}

/**
 * @brief Ремонт -> новое HealthyDevice.
 */
std::unique_ptr<Device> FaultyDeviceEx::repair(uint64_t uptimeAfterRepairSec) const {
    return std::unique_ptr<Device>(new HealthyDevice(
        name_, address_, priority_, uptimeAfterRepairSec
    ));
}
