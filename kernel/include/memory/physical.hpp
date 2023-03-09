#pragma once

#include <cstddef>
#include <cstdint>

namespace memory::physical {
static constexpr size_t page_size = 0x1000;
extern uintptr_t mem_top;

size_t total();
size_t usable();
size_t used();
size_t free();

void* alloc(size_t count = 1);
void free(void* ptr, size_t count = 1);

void init();

template <typename T = void*>
inline T alloc(size_t count = 1) {
    return reinterpret_cast<T>(alloc(count));
}

inline void free(auto ptr, size_t count) {
    return free(reinterpret_cast<void*>(ptr), count);
}
}  // namespace memory::physical