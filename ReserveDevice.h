#pragma once
#include "HealthyDevice.h"
#include <string>
#include <sstream>

/**
* @class ReserveDevice
* @brief Исправное устройство, находящееся в резерве
*
* Наследует @ref HealthyDevice и добавляет поле времени ожидания
* ввода в работу @ref standbyWaitSec_
*/

class ReserveDevice : public HealthyDevice {
public:
    /**
   * @brief Конструктор резервного устройства
   * @param name Имя устройства
   * @param addr IPадрес устройства
   * @param prio Приоритет обслуживания
   * @param uptimeSec Наработка устройства в секундах
   * @param standbyWaitSec Время ожидания ввода в работу
   */
    ReserveDevice(std::string name,
        Address addr,
        ServicePriority prio,
        uint64_t uptimeSec,
        uint64_t standbyWaitSec)
        : HealthyDevice(std::move(name), addr, prio, uptimeSec),
        standbyWaitSec_(standbyWaitSec) {
    }

    
    uint64_t standbyWait() const noexcept { return standbyWaitSec_; }

    
    std::string toString() const override;



private:
    uint64_t standbyWaitSec_{ 0 };
};
