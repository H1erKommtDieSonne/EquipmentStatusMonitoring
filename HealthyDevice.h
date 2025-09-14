#pragma once
#include "Device.h"
#include <memory>
#include <string>
#include <sstream>

//
class HealthyDevice : public Device {
public:
   //
    HealthyDevice(std::string name, Address addr, ServicePriority prio, uint64_t uptimeSec)
        : Device(std::move(name), addr, prio), uptimeSec_(uptimeSec) {
    }

   //
    bool isFaulty() const noexcept override { return false; }

   //
    uint64_t uptime() const noexcept { return uptimeSec_; }

    //
    std::unique_ptr<Device> breakDown(std::string fault) const override;

   //
    std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const override;

   //
    void setPriority(ServicePriority p) override { priority_ = p; }

   //
    std::string toString() const override {
        std::ostringstream os;
        os << "HealthyDevice{name=" << name_
            << ", addr=" << address_
            << ", prio=" << static_cast<int>(priority_)
            << ", uptimeSec=" << uptimeSec_ << "}";
        return os.str();
    }

protected:
    uint64_t uptimeSec_{ 0 };
};
