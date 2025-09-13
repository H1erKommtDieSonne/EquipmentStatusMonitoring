#include "FaultyDevice.h"
#include <cctype>
#include <vector>
#include <string>
#include <algorithm>

//full replacement after hardtests 09 09
//new commit after 3 bad tests 13 09


uint32_t FaultyDevice::ipv4_to_u32(const std::string& s) {
    //basic check
    if (s.empty()) {
        throw std::invalid_argument("empty IPv4 string");
    }

    //onlydots
    for (unsigned char ch : s) {
        if (!(std::isdigit(ch) || ch == '.')) {
            throw std::invalid_argument("invalid char in IPv4: " + s);
        }
    }
    //
    //
    //new  code 13 09
    if (std::count(s.begin(), s.end(), '.') != 3) {
        throw std::invalid_argument("IPv4 must have 3 dots: " + s);
    }
    //
    //
    //correct division
    std::vector<std::string> parts;
    parts.reserve(4);
    std::size_t pos = 0, dot;
    for (int i = 0; i < 3; ++i) {
        dot = s.find('.', pos);
        if (dot == std::string::npos) {
            throw std::invalid_argument("IPv4 must have 3 dots: " + s);
        }
        if (dot == pos) {
            throw std::invalid_argument("invalid octet in IPv4: " + s);
        }
        parts.emplace_back(s.substr(pos, dot - pos));
        pos = dot + 1;
    }
    //last octet check
    if (pos >= s.size()) {
        throw std::invalid_argument("invalid last octet in IPv4: " + s);
    }

    if (s.find('.', pos) != std::string::npos) {
        throw std::invalid_argument("too many dots in IPv4: " + s);
    }

    parts.emplace_back(s.substr(pos));

    if (parts.size() != 4) {
        throw std::invalid_argument("IPv4 must have 4 octets: " + s);
    }

    //diapazon 
    //commit 13 09
    auto parse_octet = [&](const std::string& p) -> unsigned {
        if (p.empty()) throw std::invalid_argument("empty octet");
        for (unsigned char ch : p) {
            if (!std::isdigit(ch)) {
                throw std::invalid_argument("bad octet: " + p);
            }
        }
        unsigned long val = 0;
        try {
            val = std::stoul(p, nullptr, 10);
        }
        catch (...) {
            throw std::invalid_argument("bad octet: " + p);
        }
        if (val > 255) throw std::out_of_range("octet out of range: " + p);
        return static_cast<unsigned>(val);
        };

    unsigned a = parse_octet(parts[0]);
    unsigned b = parse_octet(parts[1]);
    unsigned c = parse_octet(parts[2]);
    unsigned d = parse_octet(parts[3]);

    // end 13 09

    //gluing all
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
