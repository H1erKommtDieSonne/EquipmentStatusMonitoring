#include "FaultyDeviceEx.h"
#include "HealthyDevice.h"
#include <memory>
#include <utility>

/**
 * @brief Повторная поломка неисправного устройства
 *
 * Для уже неисправного устройства операция поломки не меняет тип и
 * возвращает дубликат текущего объекта. неисправное остаётся неисправным
 *
 * @return указатель на новый @ref FaultyDeviceEx.копия текущего
 * @note Возвращается новый объект
 */

std::unique_ptr<Device> FaultyDeviceEx::breakDown(std::string) const {
    
    return std::unique_ptr<Device>(new FaultyDeviceEx(*this));
}

/**
 * @brief Ремонт неисправного устройства
 *
 * Создаёт новый объект @ref HealthyDevice с тем же именем, адресом и приоритетом,
 * но с переданным значением наработки после ремонта
 *
 * @param uptimeAfterRepairSec Новая наработка в секундах после ремонта
 * @return указатель на новый исправный @ref HealthyDevice
 */

std::unique_ptr<Device> FaultyDeviceEx::repair(uint64_t uptimeAfterRepairSec) const {
    return std::unique_ptr<Device>(new HealthyDevice(
        name_, address_, priority_, uptimeAfterRepairSec
    ));
}
