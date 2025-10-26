#ifndef ESM_ENABLE_PYBIND
#else

//Биндинг включён
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "esm/Engine.h"
#include "FaultyDevice.h"
#include "HealthyDevice.h"
#include "ReserveDevice.h"
#include "FaultyDeviceEx.h"
#include "Device.h"
#include "ServicePriority.h"

namespace py = pybind11;

PYBIND11_MODULE(esm_py, m) {
    m.doc() = "Python bindings for Equipment Status Monitoring (ESM)";

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

    py::class_<Device, std::shared_ptr<Device>>(m, "Device")
        .def_property_readonly("name", &Device::name)
        .def_property_readonly("address", &Device::address)
        .def_property_readonly("priority", &Device::priority)
        .def("is_faulty", &Device::isFaulty)
        .def("requires_service", &Device::requiresService);

    py::class_<HealthyDevice, Device, std::shared_ptr<HealthyDevice>>(m, "HealthyDevice")
        .def(py::init<std::string, Device::Address, ServicePriority, std::uint64_t>(),
            py::arg("name"), py::arg("address"), py::arg("priority"), py::arg("uptime_sec"))
        .def_property_readonly("uptime", &HealthyDevice::uptime);

    py::class_<ReserveDevice, HealthyDevice, std::shared_ptr<ReserveDevice>>(m, "ReserveDevice")
        .def(py::init<std::string, Device::Address, ServicePriority, std::uint64_t, std::uint64_t>(),
            py::arg("name"), py::arg("address"), py::arg("priority"),
            py::arg("uptime_sec"), py::arg("standby_wait_sec"))
        .def_property_readonly("standby_wait", &ReserveDevice::standbyWait);

    py::class_<FaultyDeviceEx, Device, std::shared_ptr<FaultyDeviceEx>>(m, "FaultyDeviceEx")
        .def(py::init<std::string, Device::Address, ServicePriority, std::string>(),
            py::arg("name"), py::arg("address"), py::arg("priority"), py::arg("fault"))
        .def_property_readonly("fault", &FaultyDeviceEx::fault_description);

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

    py::class_<Engine>(m, "Engine")
        .def(py::init<const std::string&>(), py::arg("db_path"))
        .def("migrate", &Engine::migrate)
        .def("add_worker", &Engine::add_worker)
        .def("list_workers", &Engine::list_workers)
        .def("upsert_device", [](Engine& eng, const std::shared_ptr<Device>& d) { eng.upsert_device(*d); })
        .def("list_devices", &Engine::list_devices)
        .def("breakdown", &Engine::breakdown, py::arg("address"), py::arg("fault"))
        .def("start_repair", &Engine::start_repair, py::arg("job_id"), py::arg("worker_id"))
        .def("finish_repair", &Engine::finish_repair, py::arg("job_id"), py::arg("uptime_after_sec"))
        .def("list_jobs", &Engine::list_jobs, py::arg("status") = std::nullopt);

    m.def("ipv4_to_u32", &FaultyDevice::ipv4_to_u32);
    m.def("u32_to_ipv4", &FaultyDevice::u32_to_ipv4);
}
#endif
