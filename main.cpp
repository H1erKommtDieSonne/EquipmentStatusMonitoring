#include <cassert>  //������ ������ �� �������� �������
#include <vector>   //���������� ��� ������� ��������
#include <algorithm>    //����������
#include <iostream> //���� � ������� ����� ����
#include "FaultyDevice.h"

int main() {
    //��������
    auto addr = FaultyDevice::ipv4_to_u32("10.0.0.5");  //����� ������� ������
    FaultyDevice d1("�����#1", addr, ServicePriority::High, "��������");
    FaultyDevice d2("������#2", FaultyDevice::ipv4_to_u32("10.0.0.6"),
        ServicePriority::Low, "��������");

    //������� � �������������� ������
    assert(FaultyDevice::u32_to_ipv4(d1.address()) == "10.0.0.5");

    //��������� ����������
    d2.setPriority(ServicePriority::High);
    assert(d2.priority() == ServicePriority::High);

    //��������� �� ����������
    FaultyDevice a("A", addr, ServicePriority::High, "X");
    FaultyDevice b("B", addr, ServicePriority::Low, "Y");
    FaultyDevice c("C", addr, ServicePriority::None, "Z");

    assert((a <=> b) == std::strong_ordering::greater);
    assert((b <=> c) == std::strong_ordering::greater);
    assert((a == d2));

    //���������� ������ �� ��������
    std::vector<FaultyDevice> v{ b, c, a };
    std::sort(v.begin(), v.end(), std::greater<>()); //�������� ��������
    assert(v.front().priority() == ServicePriority::High);
    assert(v.back().priority() == ServicePriority::None);

    std::cout << "ALL TESTS PASSED\n"; //���� �� ����� �� ���� �� ����� ��������
    return 0;
}
