#include <cstddef>
#include <cstdint>
#include <utils/bitmap.hpp>

namespace utils {
bool bitmap_t::get(int index) const {
    uint8_t bit = index % 8;
    uint8_t byte = index / 8;

    return buffer[byte] & (1 << bit);
}

void bitmap_t::set(int index, bool value) {
    uint8_t bit = index % 8;
    uint16_t byte = index / 8;

    if (value)
        buffer[byte] |= (1 << bit);
    else
        buffer[byte] &= ~(1 << bit);
}
}  // namespace utils