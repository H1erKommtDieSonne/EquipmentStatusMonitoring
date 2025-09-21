#include "HealthyDevice.h"
#include "FaultyDeviceEx.h"
#include <memory>
#include <stdexcept>
#include <utility>

/**
 * @brief Перевести исправное устройство в состояние неисправного
 * Создаёт новый объект @ref FaultyDeviceEx с тем же именем, адресом и приоритетом
 * Если исходный приоритет равен 0, он автоматически повышается до 1
 * @param fault описание причины поломки
 * @return Указатель на новое устройство в состоянии неисправно
 */

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

/**
 * @brief Ремонт исправного устройства
 * ремонт исправного устройства эквивалентен созданию нового
 * @ref HealthyDevice с той же базовой информацией и переданной наработкой
 * @param uptimeAfterRepairSec Наработка после ремонта
 * @return Указатель на новое исправное устройство
 */

std::unique_ptr<Device> HealthyDevice::repair(uint64_t uptimeAfterRepairSec) const {
    return std::unique_ptr<Device>(new HealthyDevice(
        name_, address_, priority_, uptimeAfterRepairSec
    ));
}
