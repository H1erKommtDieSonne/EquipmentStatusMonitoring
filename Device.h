#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <sstream>

enum class ServicePriority : uint8_t;

/**
* @class Device
* @brief ����������� ���� ��� ���� ���������
* �������� ����� ���� (���, �����, ���������) � ����� ����������� ������ ���
* ����������� ��������� � ��������� ��������� � ����������� �������
*/

class Device {
public:
    using Address = uint32_t;

    //
    /**
     * @brief ����������� �������� ����������
     * @param name ��� ����������
     * @param addr IP����� ����������
     * @param prio ��������� ������������
     */

    Device(std::string name, Address addr, ServicePriority prio)
        : name_(std::move(name)), address_(addr), priority_(prio) {
    }

    virtual ~Device() = default;

    /// @name ������ � ���������
    ///@{
    /**
     * @brief ��� ����������
     * @return ������ �� ������ � ������
     */

    const std::string& name() const noexcept { return name_; }
    Address            address() const noexcept { return address_; }
    ServicePriority    priority() const noexcept { return priority_; }

    ///@}

    
    /**
     * @brief ���������� ��������� ������������
     * @param p ����� �������� ����������
     * @note ��������� � ����������� ����� ���������� � ����������� �������
     */
    
    virtual void setPriority(ServicePriority p) { priority_ = p; }

    /**
     * @brief ������� ����, ��� ���������� ����������
     * @return true, ���� ���������� ����������, ����� false
     */

    virtual bool isFaulty() const noexcept = 0;


    virtual std::unique_ptr<Device> breakDown(std::string fault) const = 0;


    virtual std::unique_ptr<Device> repair(uint64_t uptimeAfterRepairSec) const = 0;

    
    virtual std::string toString() const;

    /**
     * @brief ��������� �� ������������
     * �� ��������� ������� ��� ������������ ��������� ��� ������ ����������,
     * ��������� �� ��������
     * @return true, ���� ��������� �� ����� ����
     */

    bool requiresService() const noexcept { return priority_ != static_cast<ServicePriority>(0); }

protected:
    std::string     name_;
    Address         address_;
    ServicePriority priority_;
};
