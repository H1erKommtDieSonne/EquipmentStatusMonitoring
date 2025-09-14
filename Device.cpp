//

#include "Device.h"
#include <sstream>

 //
std::string Device::toString() const {
    std::ostringstream os;
    os << "Device{name=" << name_
        << ", addr=" << address_
        << ", prio=" << static_cast<int>(priority_)
        << "}";
    return os.str();
}
