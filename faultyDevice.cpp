#include "FaultyDevice.h"

uint32_t FaultyDevice::ipv4_to_u32(const std::string& s) {        //1 статический метод класса строка абвг в число 
    unsigned a, b, c, d;    //октеты  ipv4
    char dot1, dot2, dot3;
    std::istringstream is(s);   //поток ввода для парсинга
    if (!(is >> a >> dot1 >> b >> dot2 >> c >> dot3 >> d) ||    //проверяем что точки
        dot1 != '.' || dot2 != '.' || dot3 != '.')
        throw std::invalid_argument("неверный формат адреса ipv4 : " + s);

    if (a > 255 || b > 255 || c > 255 || d > 255)   //проверяем диапазон
        throw std::out_of_range("адрес ipv4 не в диапазоне: " + s);

    //сетевой порядок байтов
    return (static_cast<uint32_t>(a) << 24) |
        (static_cast<uint32_t>(b) << 16) |
        (static_cast<uint32_t>(c) << 8) |
        (static_cast<uint32_t>(d));
}

std::string FaultyDevice::u32_to_ipv4(uint32_t v) { //2 стат метод обратно число в строку
    std::ostringstream os;
    os << ((v >> 24) & 0xFF) << '.' //сдвиги и маски
        << ((v >> 16) & 0xFF) << '.'
        << ((v >> 8) & 0xFF) << '.'
        << (v & 0xFF);
    return os.str();
}
