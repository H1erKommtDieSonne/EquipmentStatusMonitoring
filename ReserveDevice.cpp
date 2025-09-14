//

#include "ReserveDevice.h"
#include <sstream>

 //
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