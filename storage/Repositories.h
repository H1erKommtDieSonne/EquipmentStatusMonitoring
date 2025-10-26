#pragma once
/**
 * @file Repositories.h
 * @brief Интерфейсы репозиториев поверх SqliteStorage.
 */

 // SqliteStorage — уже с устойчивым include внутри файла

#include "SqliteStorage.h"



#include "../Device.h"



#include "../ServicePriority.h"

#include <optional>
#include <vector>

/**
 * @class DeviceRepository
 * @brief CRUD-операции для таблицы devices
 */
class DeviceRepository {
public:
    explicit DeviceRepository(SqliteStorage& s) : st_(s) {}

    /// @brief Вставка/обновление устройства.
    void upsert(const Device& d);

    /// @brief Загрузка всех адресов устройств.
    std::vector<Device::Address> listAddresses() const;  // ? было без const

private:
    SqliteStorage& st_;
};