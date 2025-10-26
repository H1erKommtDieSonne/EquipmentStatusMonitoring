#include "ReserveDevice.h"
#include <sstream>

/**
 * @brief ѕредставление резервного устройства.
 */
std::string ReserveDevice::toString() const {
    std::ostringstream os;
    os << "ReserveDevice{name=" << name_
        << ", addr=" << address_
        << ", prio=" << static_cast<int>(priority_)
        << ", uptimeSec=" << uptime()
        << ", standbyWaitSec=" << standbyWait()
        << "}";
    return os.str();
}
