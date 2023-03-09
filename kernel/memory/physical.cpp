#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <arch/arch.hpp>
#include <kernel.hpp>
#include <memory/memory.hpp>
#include <memory/physical.hpp>
#include <sys/logger.hpp>
#include <sys/panic.hpp>
#include <utils/bitmap.hpp>
#include <utils/math.hpp>

namespace memory::physical {
static uintptr_t mem_usable_top = 0;
static size_t last_index = 0;
static utils::bitmap_t bitmap;

size_t usable_memory = 0;
size_t total_memory = 0;
size_t used_memory = 0;

uintptr_t mem_top = 0;

size_t total() {
    return total_memory;
}

size_t usable() {
    return usable_memory;
}

size_t used() {
    return used_memory;
}

size_t free() {
    return usable_memory - used_memory;
}

void* alloc(size_t count) {
    if (count == 0)
        return nullptr;

    auto inner_alloc = [count](size_t limit) -> void* {
        size_t p = 0;
        while (last_index < limit) {
            if (bitmap.get(last_index++) == false) {
                if (++p == count) {
                    size_t page = last_index - count;
                    for (size_t i = page; i < last_index; i++)
                        bitmap.set(i, true);
                    return reinterpret_cast<void*>(page * page_size);
                }
            } else
                p = 0;
        }
        return nullptr;
    };

    size_t i = last_index;
    void* ret = inner_alloc(mem_usable_top / page_size);

    if (ret == nullptr) {
        last_index = 0;
        ret = inner_alloc(i);
        if (ret == nullptr)
            panic::panic("Physical Memory Manager: Out of memory!");
    }

    memset(utils::tohh(ret), 0, count * page_size);
    used_memory += count * page_size;

    return ret;
}

void free(void* ptr, size_t count) {
    if (ptr == nullptr)
        return;

    size_t page = reinterpret_cast<size_t>(ptr) / page_size;

    for (size_t i = page; i < page + count; i++)
        bitmap.set(i, false);

    last_index = std::min(last_index, page);
    used_memory -= count * page_size;
}

void init() {
    logger::log_info("Initializing Physical Memory Manager");

    limine_memmap_entry** memmaps = memmap_request.response->entries;
    size_t memmap_count = memmap_request.response->entry_count;

    for (size_t i = 0; i < memmap_count; i++) {
        uintptr_t top = memmaps[i]->base + memmaps[i]->length;
        mem_top = std::max(mem_top, top);

        switch (memmaps[i]->type) {
            case LIMINE_MEMMAP_USABLE:
                usable_memory += memmaps[i]->length;
                break;
            default:
                continue;
        }

        total_memory += memmaps[i]->length;
        mem_usable_top = std::max(mem_usable_top, top);
    }

    size_t bitmap_size =
        utils::align_up((mem_usable_top / page_size) / 8, page_size);

    for (size_t i = 0; i < memmap_count; i++) {
        if (memmaps[i]->type != LIMINE_MEMMAP_USABLE)
            continue;

        if (memmaps[i]->length >= bitmap_size) {
            bitmap.reassign(
                reinterpret_cast<uint8_t*>(utils::tohh(memmaps[i]->base)),
                bitmap_size);

            memset(bitmap.get_buffer(), 0xFF, bitmap.get_size());

            memmaps[i]->length -= bitmap_size;
            memmaps[i]->base += bitmap_size;

            used_memory += bitmap_size;
            break;
        }
    }

    for (size_t i = 0; i < memmap_count; i++) {
        if (memmaps[i]->type != LIMINE_MEMMAP_USABLE)
            continue;

        for (uintptr_t t = 0; t < memmaps[i]->length; t += page_size)
            bitmap.set((memmaps[i]->base + t) / page_size, false);
    }

    // safe address + hhdm where we can map stuff
    mem_top = utils::align_up(mem_top, 0x40000000);
    memory::allocator.initialize();
}
}  // namespace memory::physical