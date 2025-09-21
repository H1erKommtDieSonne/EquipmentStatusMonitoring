#include "ReserveDevice.h"
#include <sstream>

/**
 * @brief представление резервного устройства
 * @return Строка с описанием состояния резервного устройства
 */

std::string ReserveDevice::toString() const {
    std::ostringstream os;
    os << "ReserveDevice{name=" << name_
        << ", addr=" << address_
        << ", prio=" << static_cast<int>(priority_)
        << ", uptimeSec=" << uptimeSec_
        << ", standbyWaitSec=" << standbyWaitSec_
        << "}";
    return os.str();
}