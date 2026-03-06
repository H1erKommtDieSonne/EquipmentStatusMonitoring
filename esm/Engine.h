#pragma once
/**
 * @file Engine.h
 * @brief Публичный фасад библиотеки ESM: БД, операции и выборки для UI
 */

#include "export.h"
#include "../storage/SqliteStorage.h"
#include "../storage/Repositories.h" 
#include "../storage/WorkerRepository.h"
#include "../FaultyDevice.h" 
#include "../Device.h"
#include "../HealthyDevice.h"
#include "../ReserveDevice.h"
#include "../FaultyDeviceEx.h"
#include "../ServicePriority.h"
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

 /**
  * @brief Статус задания
  */
enum class JobStatus : std::uint8_t { Open = 0, InProgress = 1, Done = 2, Canceled = 3 };

/**
 * @brief  запись оборудование для  UI
 */
struct DeviceRow {
    std::uint32_t   address{0};
    std::string     name{};
    ServicePriority priority{ServicePriority::None};
    bool            is_faulty{false};
    bool            is_reserve{false};
    std::uint64_t   uptime_sec{0};
    std::uint64_t   standby_wait_sec{0};
};

/**
 * @brief  запись работник для UI
 * 
 */
struct WorkerFlat {
    std::int64_t id{0};
    std::string  name{};
    int          max_jobs{0};
    std::string  skill{};
};

/**
 * @brief запись задача ремонта
 */
struct JobRow {
    std::int64_t   id;
    std::uint32_t  device_address;
    std::optional<std::int64_t> worker_id;
    std::string    fault;
    JobStatus      status;
    std::int64_t   created_at;
    std::optional<std::int64_t> started_at;
    std::optional<std::int64_t> finished_at;
};

/**
 * @class Engine
 * @brief Единая точка входа: миграции, операции и выборки для Python/UI
 * Внутри содержит SqliteStorage и использует репозитории
 */
class ESM_API Engine {
public:
    /// @brief Открыть/создать БД по пути
    explicit Engine(const std::string& db_path);

    /// @brief Применить встроенные миграции (создать таблицы)
    void migrate();

    //Workers
    std::int64_t add_worker(const std::string& name, int max_jobs, const std::string& skill);
    std::vector<WorkerFlat> list_workers() const;

    // Devices
    /// @brief upsert любого устройства из иерархии Device
    void upsert_device(const Device& d);
    /// @brief Плоская выборка устройств
    std::vector<DeviceRow> list_devices() const;

    //Jobs
    /// @brief Зафиксировать поломку
    std::int64_t breakdown(std::uint32_t address, const std::string& fault);
    /// @brief Начать ремонт
    void start_repair(std::int64_t job_id, std::int64_t worker_id);
    /// @brief Завершить ремонт
    void finish_repair(std::int64_t job_id, std::uint64_t uptime_after_sec);

    /// @brief Выборка задач.
    std::vector<JobRow> list_jobs(std::optional<JobStatus> status = std::nullopt) const;

private:
    SqliteStorage        storage_;
    DeviceRepository     dev_repo_;
    WorkerRepository     worker_repo_;
};
