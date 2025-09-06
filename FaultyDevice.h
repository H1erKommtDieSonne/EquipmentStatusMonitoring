#pragma once    //����� ����������� 1 ���
#include <string>   //��� �����
#include <compare>  //�������� <=>(������������� �������� ���������_) ������ �� �++20
#include <cstdint>  //��� ������
#include <stdexcept>    //��� ����������
#include <sstream>  //��� �������

///��������� ������������: High > Low > None
enum class ServicePriority : uint8_t { None = 0, Low = 1, High = 2 };   //�������������� ������������

class FaultyDevice { //�������� �����
public:     //��� ������� �� ������
    FaultyDevice(std::string name,
        uint32_t address,
        ServicePriority prio,
        std::string fault)
        : name_(std::move(name)),   //�������� ���������� ��� �����(�����������)
        address_(address),  //������� ����� 32 ����
        priority_(prio),    //���������
        fault_(std::move(fault)) {  //�������� �������������
    }

    //�������
    const std::string& name() const noexcept { return name_; }  //�� ��������� ������ � �� ������� ����������
    uint32_t address() const noexcept { return address_; }
    ServicePriority priority() const noexcept { return priority_; }
    const std::string& fault_description() const noexcept { return fault_; }

    //�������
    void setPriority(ServicePriority newPrio) noexcept { priority_ = newPrio; } //������ ������ ���������
    void setFault(std::string newFault) { fault_ = std::move(newFault); }   //��������� ����� �������������

    //��������� �� ����������
    //��� ���� ��������� ��� ������ ������
    std::strong_ordering operator<=>(const FaultyDevice& rhs) const noexcept {
        int lhsRank = rank(priority_);  //��������� � �����
        int rhsRank = rank(rhs.priority_);
        if (lhsRank != rhsRank) return lhsRank <=> rhsRank;

        //��� ������ �����������
        if (name_ != rhs.name_) return name_ <=> rhs.name_;
        return address_ <=> rhs.address_;
    }

    //��������� �� ����������
    bool operator==(const FaultyDevice& rhs) const noexcept {
        return priority_ == rhs.priority_;
    }

    //������� ��� IPv4
    // ����������� ������ 
    //�������������� �� a.b.c.d � 32������ �����
    static uint32_t ipv4_to_u32(const std::string& dotted);
    static std::string u32_to_ipv4(uint32_t value);

private:    //�������� ��� ������� ����������
    static constexpr int rank(ServicePriority p) noexcept {
        switch (p) {
        case ServicePriority::High: return 3;
        case ServicePriority::Low:  return 2;
        case ServicePriority::None: return 1;
        }
        return 0;
    }
    //����
    std::string name_;
    uint32_t    address_;
    ServicePriority priority_;
    std::string fault_;
};
