#pragma once

#include <cstddef>
#include <cstdint>

namespace utils {
class bitmap_t {
   public:
    bitmap_t() : buffer(nullptr), size(0) {}

    bitmap_t(uint8_t* buff, size_t size) : buffer(buff), size(size) {}

    void reassign(uint8_t* _buff, size_t _size) {
        buffer = _buff;
        size = _size;
    }

    size_t get_size() { return size; }

    uint8_t* get_buffer() { return buffer; }

    bool get(int index) const;
    void set(int index, bool value);

   private:
    uint8_t* buffer;
    size_t size;
};
}  // namespace utils