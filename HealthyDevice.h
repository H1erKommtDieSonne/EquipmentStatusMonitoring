#pragma once
#include "Device.h"
#include <memory>
#include <string>
#include <sstream>

/**
* @class HealthyDevice
* @brief ��������� ����������(��������� @ref Device)
* ������������� ������ ��������� � ��������(@ref uptimeSec_) �
* ������������� ������ @ref uptime.
* ������������ ��������:
*-@ref breakDown � ������� � ����������
*-@ref repair � ������(�������� ������ ���������� ����������)
*/

class HealthyDevice : public Device {
public:
    /**
  * @brief ����������� ���������� ����������.
  * @param name ��� ����������
  * @param addr IP�����
  * @param prio ��������� ������������
  * @param uptimeSec ��������� � ��������
  */
    HealthyDevice(std::string name, Address addr, ServicePriority prio, uint64_t uptimeSec)
        : Device(std::move(name), addr, prio), uptimeSec_(uptimeSec) {
    }

    /**
  * @brief ������� �������������.
  * @return ������ false ��� ���������� ����������
  */

    bool isFaulty() const noexcept override { return false; }

    /**
     * @brief ������� ��������� ����������
     * @return ��������� � ��������
     */
    uint64_t uptime() const noexcept { return uptimeSec_; }

    /**
  * @brief ��������� ���������� � ��������� ����������
  * @param fault �������� ������� �������
  * @return ��������� �� ����� ���������� � ��������� ����������
  */
    std::unique_ptr<Device> breakDown(std::string fault) const override;

    /**
    * @brief ������ (�������� ������ ���������� ����������)
    * @param uptimeAfterRepairSec ��������� ����� �������
    * @return ��������� �� ����� ��������� ����������
    */
    std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const override;

    /**
    * @brief ���������� ��������� ������������
    * @param p ����� �������� ����������
    */
    void setPriority(ServicePriority p) override { priority_ = p; }

   
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
