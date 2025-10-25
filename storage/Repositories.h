#pragma once
/**
 * @file Repositories.h
 * @brief Интерфейсы репозиториев поверх SqliteStorage.
 */

 // SqliteStorage — уже с устойчивым include внутри файла
#if __has_include("storage/SqliteStorage.h")
#include "storage/SqliteStorage.h"
#elif __has_include("../storage/SqliteStorage.h")
#include "../storage/SqliteStorage.h"
#else
#include "SqliteStorage.h"
#endif

// Device.h и ServicePriority.h могут лежать уровнем выше — поддержим оба случая
#if __has_include("Device.h")
#include "Device.h"
#elif __has_include("../Device.h")
#include "../Device.h"
#else
#error "Device.h not found in include paths."
#endif

#if __has_include("ServicePriority.h")
#include "ServicePriority.h"
#elif __has_include("../ServicePriority.h")
#include "../ServicePriority.h"
#endif

#include <vector>

/**
 * @class DeviceRepository
 * @brief CRUD-операции для таблицы devices.
 */
class DeviceRepository {
public:
    /// @brief Конструктор.
    explicit DeviceRepository(SqliteStorage& s) : st_(s) {}

    /// @brief Вставка/обновление устройства.
    void upsert(const Device& d);

    /// @brief Загрузка всех адресов устройств.
    std::vector<Device::Address> listAddresses();

private:
    SqliteStorage& st_;
};