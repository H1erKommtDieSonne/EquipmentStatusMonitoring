п»ї#include "FaultyDevice.h"

uint32_t FaultyDevice::ipv4_to_u32(const std::string& s) {        //1 СЃС‚Р°С‚РёС‡РµСЃРєРёР№ РјРµС‚РѕРґ РєР»Р°СЃСЃР° СЃС‚СЂРѕРєР° Р°Р±РІРі РІ С‡РёСЃР»Рѕ 
    unsigned a, b, c, d;    //РѕРєС‚РµС‚С‹  ipv4
    char dot1, dot2, dot3;
    std::istringstream is(s);   //РїРѕС‚РѕРє РІРІРѕРґР° РґР»СЏ РїР°СЂСЃРёРЅРіР°
    if (!(is >> a >> dot1 >> b >> dot2 >> c >> dot3 >> d) ||    //РїСЂРѕРІРµСЂСЏРµРј С‡С‚Рѕ С‚РѕС‡РєРё
        dot1 != '.' || dot2 != '.' || dot3 != '.')
        throw std::invalid_argument("РЅРµРІРµСЂРЅС‹Р№ С„РѕСЂРјР°С‚ Р°РґСЂРµСЃР° ipv4 : " + s);

    if (a > 255 || b > 255 || c > 255 || d > 255)   //РїСЂРѕРІРµСЂСЏРµРј РґРёР°РїР°Р·РѕРЅ
        throw std::out_of_range("Р°РґСЂРµСЃ ipv4 РЅРµ РІ РґРёР°РїР°Р·РѕРЅРµ: " + s);

    //СЃРµС‚РµРІРѕР№ РїРѕСЂСЏРґРѕРє Р±Р°Р№С‚РѕРІ
    return (static_cast<uint32_t>(a) << 24) |
        (static_cast<uint32_t>(b) << 16) |
        (static_cast<uint32_t>(c) << 8) |
        (static_cast<uint32_t>(d));
}

std::string FaultyDevice::u32_to_ipv4(uint32_t v) { //2 СЃС‚Р°С‚ РјРµС‚РѕРґ РѕР±СЂР°С‚РЅРѕ С‡РёСЃР»Рѕ РІ СЃС‚СЂРѕРєСѓ
    std::ostringstream os;
    os << ((v >> 24) & 0xFF) << '.' //СЃРґРІРёРіРё Рё РјР°СЃРєРё
        << ((v >> 16) & 0xFF) << '.'
        << ((v >> 8) & 0xFF) << '.'
        << (v & 0xFF);
    return os.str();
}
