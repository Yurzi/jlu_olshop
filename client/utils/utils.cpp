#include "utils.h"

namespace yolspc {

uint32_t GetThreadId() {
    std::stringstream oss;
    oss << std::this_thread::get_id();
    std::string stld = oss.str();
    return std::stoull(stld);
}

uint32_t GetFiberId() {
    return 0;
}

bool GetEndian() {
    uint32_t a = 0x12345678;
    char *p    = (char *)&a;
    if (p[0] == 0x12) {
        return false;
    } else {
        return true;
    }
}

uint32_t LittleToBig(uint32_t data) {
    return ((data << 24) | ((data & 0x0000FF00) << 8) | ((data & 0x00FF0000) >> 8) |
            (data >> 24));
}

uint32_t BigToLittle(uint32_t data) {
    return ((data << 24) | ((data & 0x0000FF00) << 8) | ((data & 0x00FF0000) >> 8) |
            (data >> 24));
}

}   // namespace yolspc
