#pragma once
#include "storage/SqliteStorage.h"
#include "Device.h"
#include "ServicePriority.h"
#include <vector>
#include <optional>

/**
 * @class DeviceRepository
 * @brief CRUD-операции для таблицы devices
 */
class DeviceRepository {
public:
    /// @brief Конструктор
    explicit DeviceRepository(SqliteStorage& s) : st_(s) {}

    /**
     * @brief Вставка/обновление устройства
     * @param d Ссылка на устройство
     */
    void upsert(const Device& d);

    /**
     * @brief Загрузка всех адресов устройств
     * @return Вектор адресов
     */
    std::vector<Device::Address> listAddresses();

private:
    SqliteStorage& st_;
};
