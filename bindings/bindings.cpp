#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../esm/Engine.h"
#include "../FaultyDevice.h"
#include "../HealthyDevice.h"
#include "../ReserveDevice.h"
#include "../FaultyDeviceEx.h"
#include "../Device.h"
#include "../ServicePriority.h"

namespace py = pybind11;

PYBIND11_MODULE(equipment_status_monitoring, m) {
    m.def("version", []() { return "0.1.0"; });

    m.doc() = "Equipment Status Monitoring bindings";

    // ===== Enums =====
    py::enum_<ServicePriority>(m, "ServicePriority")
        .value("None", ServicePriority::None)
        .value("Low", ServicePriority::Low)
        .value("High", ServicePriority::High)
        .export_values();

    py::enum_<JobStatus>(m, "JobStatus")
        .value("Open", JobStatus::Open)
        .value("InProgress", JobStatus::InProgress)
        .value("Done", JobStatus::Done)
        .value("Canceled", JobStatus::Canceled)
        .export_values();

    // ===== Device base =====
    py::class_<Device>(m, "Device")
        .def_property_readonly("name", &Device::name)
        .def_property_readonly("address", &Device::address)
        .def_property_readonly("priority", &Device::priority)
        .def("set_priority", &Device::setPriority)
        .def("is_faulty", &Device::isFaulty)
        .def("is_reserve", &Device::isReserve)
        .def("requires_service", &Device::requiresService)
        .def("__str__", &Device::toString);

    // ===== HealthyDevice =====
    py::class_<HealthyDevice, Device>(m, "HealthyDevice")
        .def(py::init<std::string, Device::Address, ServicePriority, uint64_t>(),
            py::arg("name"), py::arg("address"), py::arg("priority"), py::arg("uptime_sec"))
        .def_property_readonly("uptime", &HealthyDevice::uptime);

    // ===== ReserveDevice =====
    py::class_<ReserveDevice, HealthyDevice>(m, "ReserveDevice")
        .def(py::init<std::string, Device::Address, ServicePriority, uint64_t, uint64_t>(),
            py::arg("name"), py::arg("address"), py::arg("priority"),
            py::arg("uptime_sec"), py::arg("standby_wait_sec"))
        .def_property_readonly("standby_wait", &ReserveDevice::standbyWait);

    // ===== FaultyDeviceEx =====
    py::class_<FaultyDeviceEx, Device>(m, "FaultyDeviceEx")
        .def(py::init<std::string, Device::Address, ServicePriority, std::string>(),
            py::arg("name"), py::arg("address"), py::arg("priority"), py::arg("fault"))
        .def_property_readonly("fault", &FaultyDeviceEx::fault_description);

    // ===== POD structs =====
    py::class_<DeviceRow>(m, "DeviceRow")
        .def_readonly("address", &DeviceRow::address)
        .def_readonly("name", &DeviceRow::name)
        .def_readonly("priority", &DeviceRow::priority)
        .def_readonly("is_faulty", &DeviceRow::is_faulty)
        .def_readonly("is_reserve", &DeviceRow::is_reserve)
        .def_readonly("uptime_sec", &DeviceRow::uptime_sec)
        .def_readonly("standby_wait_sec", &DeviceRow::standby_wait_sec);

    py::class_<WorkerFlat>(m, "WorkerFlat")
        .def_readonly("id", &WorkerFlat::id)
        .def_readonly("name", &WorkerFlat::name)
        .def_readonly("max_jobs", &WorkerFlat::max_jobs)
        .def_readonly("skill", &WorkerFlat::skill);

    py::class_<JobRow>(m, "JobRow")
        .def_readonly("id", &JobRow::id)
        .def_readonly("device_address", &JobRow::device_address)
        .def_readonly("worker_id", &JobRow::worker_id)
        .def_readonly("fault", &JobRow::fault)
        .def_readonly("status", &JobRow::status)
        .def_readonly("created_at", &JobRow::created_at)
        .def_readonly("started_at", &JobRow::started_at)
        .def_readonly("finished_at", &JobRow::finished_at);

    // ===== Engine =====
    py::class_<Engine>(m, "Engine")
        .def(py::init<const std::string&>(), py::arg("db_path"))
        .def("migrate", &Engine::migrate)

        // workers
        .def("add_worker", &Engine::add_worker,
            py::arg("name"), py::arg("max_jobs"), py::arg("skill"))
        .def("list_workers", &Engine::list_workers)

        // devices
        .def("upsert_device", &Engine::upsert_device)
        .def("list_devices", &Engine::list_devices)

        // jobs
        .def("breakdown", &Engine::breakdown)
        .def("start_repair", &Engine::start_repair)
        .def("finish_repair", &Engine::finish_repair)
        .def("list_jobs", &Engine::list_jobs,
            py::arg("status") = std::nullopt);
}
