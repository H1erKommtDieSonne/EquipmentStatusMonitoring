#pragma once
#include "HealthyDevice.h"
#include <string>
#include <sstream>

/**
* @class ReserveDevice
* @brief Исправное устройство, находящееся в резерве.
*/
class ReserveDevice : public HealthyDevice {
public:
    ReserveDevice(std::string name,
        Address addr,
        ServicePriority prio,
        uint64_t uptimeSec,
        uint64_t standbyWaitSec)
        : HealthyDevice(std::move(name), addr, prio, uptimeSec),
        standbyWaitSec_(standbyWaitSec) {
    }

    /// @brief Полиморфное копирование.
    std::unique_ptr<Device> clone() const override {
        return std::unique_ptr<Device>(new ReserveDevice(*this));
    }

    /// @brief Резервное устройство.
    bool isReserve() const noexcept override { return true; }

    /// @brief Время ожидания ввода в работу (секунды).
    uint64_t standbyWait() const noexcept { return standbyWaitSec_; }

    std::string toString() const override;

private:
    uint64_t standbyWaitSec_{ 0 };
};
