#pragma once
#include "HealthyDevice.h"
#include <string>
#include <sstream>

/**
* @class ReserveDevice
* @brief ��������� ����������, ����������� � �������
*
* ��������� @ref HealthyDevice � ��������� ���� ������� ��������
* ����� � ������ @ref standbyWaitSec_
*/

class ReserveDevice : public HealthyDevice {
public:
    /**
   * @brief ����������� ���������� ����������
   * @param name ��� ����������
   * @param addr IP����� ����������
   * @param prio ��������� ������������
   * @param uptimeSec ��������� ���������� � ��������
   * @param standbyWaitSec ����� �������� ����� � ������
   */
    ReserveDevice(std::string name,
        Address addr,
        ServicePriority prio,
        uint64_t uptimeSec,
        uint64_t standbyWaitSec)
        : HealthyDevice(std::move(name), addr, prio, uptimeSec),
        standbyWaitSec_(standbyWaitSec) {
    }

    
    uint64_t standbyWait() const noexcept { return standbyWaitSec_; }

    
    std::string toString() const override;



private:
    uint64_t standbyWaitSec_{ 0 };
};
