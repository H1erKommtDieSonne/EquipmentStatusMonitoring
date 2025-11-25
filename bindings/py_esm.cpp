/**
 * @file py_esm.cpp
 * @brief Boost.Python-обёртка
 *
 * Модуль экспонирует:
 *  - перечисления ServicePriority и JobStatus;
 *  - иерархию устройств (Device, HealthyDevice, ReserveDevice, FaultyDeviceEx);
 *  - структуры DeviceRow, WorkerFlat, JobRow;
 *  - фасад Engine;
 *  - вспомогательные функции для работы с адресами IPv4.
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/python.hpp>

#include "../esm/Engine.h"
#include "../FaultyDevice.h"
#include "../HealthyDevice.h"
#include "../ReserveDevice.h"
#include "../FaultyDeviceEx.h"
#include "../Device.h"
#include "../ServicePriority.h"

namespace py = boost::python;


/**
 * @brief Обёртка для Engine::upsert_device, принимающая shared_ptr<Device>.
 *
 * @param engine Ссылка на фасад ESM.
 * @param dev Умный указатель на устройство.
 */
static void engine_upsert_device(Engine& engine,
    const std::shared_ptr<Device>& dev)
{
    engine.upsert_device(*dev);
}

/**
 * @brief Преобразовать std::vector<WorkerFlat> в Python-список.
 *
 * @param engine Ссылка на фасад ESM.
 * @return Python-список объектов WorkerFlat.
 */
static py::list engine_list_workers(const Engine& engine)
{
    py::list result;
    const std::vector<WorkerFlat> workers = engine.list_workers();
    for (const auto& w : workers)
    {
        result.append(w);
    }
    return result;
}

/**
 * @brief Преобразовать std::vector<DeviceRow> в Python-список.
 *
 * @param engine Ссылка на фасад ESM.
 * @return Python-список объектов DeviceRow.
 */
static py::list engine_list_devices(const Engine& engine)
{
    py::list result;
    const std::vector<DeviceRow> devices = engine.list_devices();
    for (const auto& d : devices)
    {
        result.append(d);
    }
    return result;
}

/**
 * @brief Обёртка для Engine::list_jobs
 *
 * @param engine Ссылка на фасад ESM
 * @param status_obj Объект Python: либо None, либо JobStatus.
 * @return Python-список объектов JobRow.
 */
static py::list engine_list_jobs(const Engine& engine, py::object status_obj)
{
    std::optional<JobStatus> status;

    if (!status_obj.is_none())
    {
        status = py::extract<JobStatus>(status_obj);
    }

    py::list result;
    const std::vector<JobRow> jobs = engine.list_jobs(status);
    for (const auto& j : jobs)
    {
        result.append(j);
    }
    return result;
}

/**
 * @brief Получить worker_id как Python-объект
 *
 * @param row Строка таблицы задач.
 * @return Идентификатор работника или None.
 */
static py::object jobrow_worker_id(const JobRow& row)
{
    if (row.worker_id.has_value())
    {
        return py::object(*row.worker_id);
    }
    return py::object(); // None
}

/**
 * @brief Получить started_at как Python-объект
 *
 * @param row Строка таблицы задач.
 * @return Время начала или None.
 */
static py::object jobrow_started_at(const JobRow& row)
{
    if (row.started_at.has_value())
    {
        return py::object(*row.started_at);
    }
    return py::object();
}

/**
 * @brief Получить finished_at как Python-объект
 *
 * @param row Строка таблицы задач.
 * @return Время завершения или None.
 */
static py::object jobrow_finished_at(const JobRow& row)
{
    if (row.finished_at.has_value())
    {
        return py::object(*row.finished_at);
    }
    return py::object(); // None
}

/**
 * @brief Точка входа модуля Boost.Python esm_py
 */
BOOST_PYTHON_MODULE(esm_py)
{

    py::enum_<ServicePriority>("ServicePriority",
        "Приоритет обслуживания устройства.")
        .value("None", ServicePriority::None)
        .value("Low", ServicePriority::Low)
        .value("High", ServicePriority::High)
        .export_values();

    py::enum_<JobStatus>("JobStatus",
        "Статус ремонтной заявки.")
        .value("Open", JobStatus::Open)
        .value("InProgress", JobStatus::InProgress)
        .value("Done", JobStatus::Done)
        .value("Canceled", JobStatus::Canceled)
        .export_values();

    // Базовый класс устройства

    py::class_<Device, std::shared_ptr<Device>, boost::noncopyable>(
        "Device",
        "Базовый класс устройства.",
        py::no_init)
        .add_property("name",
            &Device::name,
            "Имя устройства.")
        .add_property("address",
            &Device::address,
            "Сетевой адрес устройства.")
        .add_property("priority",
            &Device::priority,
            "Приоритет обслуживания.")
        .def("is_faulty",
            &Device::isFaulty,
            "Признак того, что устройство неисправно.")
        .def("requires_service",
            &Device::requiresService,
            "Нуждается ли устройство в обслуживании.");


    // HealthyDevice

    py::class_<HealthyDevice, Device, std::shared_ptr<HealthyDevice>>(
        "HealthyDevice",
        "Исправное устройство с учётом наработки.",
        py::init<std::string, Device::Address, ServicePriority, std::uint64_t>(
            (py::arg("name"),
                py::arg("address"),
                py::arg("priority"),
                py::arg("uptime_sec")),
            "Создать исправное устройство."))
        .add_property("uptime",
            &HealthyDevice::uptime,
            "Наработка устройства в секундах.");


    // ReserveDevice

    py::class_<ReserveDevice, HealthyDevice, std::shared_ptr<ReserveDevice>>(
        "ReserveDevice",
        "Резервное устройство.",
        py::init<std::string,
        Device::Address,
        ServicePriority,
        std::uint64_t,
        std::uint64_t>(
            (py::arg("name"),
                py::arg("address"),
                py::arg("priority"),
                py::arg("uptime_sec"),
                py::arg("standby_wait_sec")),
            "Создать резервное устройство."))
        .add_property("standby_wait",
            &ReserveDevice::standbyWait,
            "Время ожидания ввода в строй в секундах.");


    // FaultyDeviceEx

    py::class_<FaultyDeviceEx, Device, std::shared_ptr<FaultyDeviceEx>>(
        "FaultyDeviceEx",
        "Неисправное устройство с описанием неисправности.",
        py::init<std::string,
        Device::Address,
        ServicePriority,
        std::string>(
            (py::arg("name"),
                py::arg("address"),
                py::arg("priority"),
                py::arg("fault")),
            "Создать неисправное устройство."))
        .add_property("fault",
            &FaultyDeviceEx::fault_description,
            "Текстовое описание неисправности.");


    // Структуры для UI

    py::class_<DeviceRow>("DeviceRow",
        "Плоское представление устройства для UI.")
        .def_readonly("address", &DeviceRow::address)
        .def_readonly("name", &DeviceRow::name)
        .def_readonly("priority", &DeviceRow::priority)
        .def_readonly("is_faulty", &DeviceRow::is_faulty)
        .def_readonly("is_reserve", &DeviceRow::is_reserve)
        .def_readonly("uptime_sec", &DeviceRow::uptime_sec)
        .def_readonly("standby_wait_sec", &DeviceRow::standby_wait_sec);

    py::class_<WorkerFlat>("WorkerFlat",
        "Плоское представление работника для UI.")
        .def_readonly("id", &WorkerFlat::id)
        .def_readonly("name", &WorkerFlat::name)
        .def_readonly("max_jobs", &WorkerFlat::max_jobs)
        .def_readonly("skill", &WorkerFlat::skill);

    py::class_<JobRow>("JobRow",
        "Запись ремонтной задачи.")
        .def_readonly("id", &JobRow::id)
        .def_readonly("device_address", &JobRow::device_address)
        .add_property("worker_id",
            &jobrow_worker_id,
            "Идентификатор работника или None.")
        .def_readonly("fault", &JobRow::fault)
        .def_readonly("status", &JobRow::status)
        .def_readonly("created_at", &JobRow::created_at)
        .add_property("started_at",
            &jobrow_started_at,
            "Момент начала ремонта или None.")
        .add_property("finished_at",
            &jobrow_finished_at,
            "Момент завершения ремонта или None.");


    py::class_<Engine>("Engine",
        "Фасад ESM: миграции БД и высокоуровневые операции.",
        py::init<const std::string&>(py::arg("db_path")))
        .def("migrate",
            &Engine::migrate,
            "Применить миграции и создать таблицы при необходимости.")
        .def("add_worker",
            &Engine::add_worker,
            (py::arg("name"), py::arg("max_jobs"), py::arg("skill")),
            "Добавить нового работника.")
        .def("list_workers",
            &engine_list_workers,
            "Получить список работников.")
        .def("upsert_device",
            &engine_upsert_device,
            py::arg("device"),
            "Добавить или обновить устройство.")
        .def("list_devices",
            &engine_list_devices,
            "Получить список устройств.")
        .def("breakdown",
            &Engine::breakdown,
            (py::arg("address"), py::arg("fault")),
            "Зафиксировать поломку и создать задачу.")
        .def("start_repair",
            &Engine::start_repair,
            (py::arg("job_id"), py::arg("worker_id")),
            "Начать ремонт по задаче.")
        .def("finish_repair",
            &Engine::finish_repair,
            (py::arg("job_id"), py::arg("uptime_after_sec")),
            "Завершить ремонт по задаче и обновить состояние устройства.")
        .def("list_jobs",
            &engine_list_jobs,
            (py::arg("status") = py::object()),
            "Получить список задач, опционально фильтруя по статусу.");

    // Свободные функции для работы с IPv4

    py::def("ipv4_to_u32",
        &FaultyDevice::ipv4_to_u32,
        py::arg("ipv4"),
        "Преобразовать IPv4-адрес в 32‑битное целое.");

    py::def("u32_to_ipv4",
        &FaultyDevice::u32_to_ipv4,
        py::arg("value"),
        "Преобразовать 32‑битное целое в строку IPv4.");
}
