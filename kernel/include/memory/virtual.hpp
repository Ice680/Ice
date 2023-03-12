#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#define fold(x) __builtin_constant_p(x) ? x : x

namespace memory::paging {
enum flags {
    READ = 1,
    WRITE = 2,
    RW = 3,
    EXEC = 4,
    RWX = 7,
    USER = 8,
    RWU = 11,
    RWXU = 15,
    GLOBAL = 16,
    LPAGE = 32,
    LLPAGE = 64,
};

#define is_lpage(x) ({x & LPAGE || x & LLPAGE;})

enum caching {
    UNCACHABLE,
    WRITE_THROUGH,
    WRITE_PROTECTED,
    WRITE_COMBINING,
    WRITE_BACK,
    MMIO = UNCACHABLE,
    FRAMEBUFFER = WRITE_COMBINING
};

constexpr inline caching default_caching = WRITE_BACK;
constexpr inline size_t default_flags = RWX;

constexpr inline size_t KiB4 = 0x1000;
constexpr inline size_t MiB2 = 0x200000;
constexpr inline size_t GiB1 = 0x40000000;

constexpr inline uintptr_t invalid_addr = uintptr_t(-1);

extern uintptr_t pa_mask;

struct pt_entry_t {
    uintptr_t value = 0;

    void set_flags(uintptr_t flags, bool enabled) {
        uintptr_t temp = this->value;

        temp &= ~flags;
        if (enabled)
            temp |= flags;

        this->value = temp;
    }

    bool get_flags(uintptr_t flags) {
        return (this->value & flags) ? true : false;
    }

    uintptr_t get_flags() { return this->value & ~pa_mask; }

    uintptr_t get_address() { return this->value & pa_mask; }

    void set_address(uintptr_t address) {
        uintptr_t temp = this->value;

        temp &= ~pa_mask;
        temp |= address;

        this->value = temp;
    }
};

namespace memmap {
struct global;
struct local;

constexpr inline void* map_failed = fold(reinterpret_cast<void*>(-1));

constexpr inline int map_shared = 0x01;
constexpr inline int map_private = 0x02;
constexpr inline int map_shared_validate = 0x03;
constexpr inline int map_type = 0x0F;
constexpr inline int map_fixed = 0x10;
constexpr inline int map_anon = 0x20;
constexpr inline int map_anonymous = map_anon;
constexpr inline int map_noreserve = 0x4000;
constexpr inline int map_growsdown = 0x0100;
constexpr inline int map_denywrite = 0x0800;
constexpr inline int map_executable = 0x1000;
constexpr inline int map_locked = 0x2000;
constexpr inline int map_populate = 0x8000;
constexpr inline int map_nonblock = 0x10000;
constexpr inline int map_stack = 0x20000;
constexpr inline int map_hugetlb = 0x40000;
constexpr inline int map_sync = 0x80000;
constexpr inline int map_fixed_noreplace = 0x100000;
constexpr inline int map_file = 0;

constexpr inline int protocol_none = 0;
constexpr inline int protocol_read = 1;
constexpr inline int protocol_write = 2;
constexpr inline int protocol_exec = 4;
constexpr inline int protocol_growsdown = 0x01000000;
constexpr inline int protocol_growsup = 0x02000000;
}  // namespace memmap

struct page_table_t;

struct pagemap_t {
    std::vector<std::shared_ptr<memmap::local>> ranges;
    page_table_t* top_level = nullptr;

    size_t llpage_size = 0;
    size_t lpage_size = 0;
    size_t page_size = 0;

    inline size_t get_page_size(size_t flags) {
        size_t page_size = this->page_size;

        if (flags & LPAGE)
            page_size = this->lpage_size;
        if (flags & LLPAGE)
            page_size = this->llpage_size;

        return page_size;
    }

    inline std::pair<size_t, size_t> required_size(size_t size) {
        if (size >= this->llpage_size)
            return {this->llpage_size, LLPAGE};
        else if (size >= this->lpage_size)
            return {this->lpage_size, LPAGE};

        return {this->page_size, 0};
    }

    std::optional<
        std::tuple<std::shared_ptr<memmap::local>, size_t, size_t>>
    address_to_range(uintptr_t address);

    pt_entry_t* virtual_to_pt_entry(uint64_t virtual_address, bool allocate,
                                    uint64_t page_size);
    uintptr_t virtual_to_physical(uintptr_t virtual_address, size_t flags = 0);

    bool map(uintptr_t virtual_address, uintptr_t physical_address,
             size_t flags = default_flags, caching cache = default_caching);
    bool unmap(uintptr_t virtual_address, size_t flags = 0);
    bool set_flags(uintptr_t virtual_address, size_t flags = default_flags,
                   caching cache = default_caching);

    inline bool remap(uintptr_t old_ptr, uintptr_t new_ptr,
                      size_t flags = default_flags,
                      caching cache = default_caching) {
        uint64_t physical_address = this->virtual_to_physical(old_ptr, flags);

        this->unmap(old_ptr, flags);

        return this->map(new_ptr, physical_address, flags, cache);
    }

    inline void map_range(uintptr_t virtual_address, uintptr_t physical_address,
                          size_t size, size_t flags = default_flags,
                          caching cache = default_caching) {
        size_t page_size = this->get_page_size(flags);

        for (size_t i = 0; i < size; i += page_size)
            this->unmap(virtual_address + i, flags);
    }

    inline void unmap_range(uintptr_t virtual_address, size_t size,
                            size_t flags = 0) {
        size_t page_size = this->get_page_size(flags);

        for (size_t i = 0; i < size; i += page_size)
            this->unmap(virtual_address + i, flags);
    }

    inline void remap_range(uintptr_t old_ptr, uintptr_t new_ptr, size_t size,
                            size_t flags = default_flags,
                            caching cache = default_caching) {
        size_t page_size = this->get_page_size(flags);

        for (size_t i = 0; i < size; i += page_size)
            this->remap(old_ptr + i, new_ptr + i, flags, cache);
    }

    inline void set_flags_range(uintptr_t virtual_address, size_t size,
                                size_t flags = default_flags,
                                caching cache = default_caching) {
        size_t page_size = this->get_page_size(flags);

        for (size_t i = 0; i < size; i += page_size)
            this->set_flags(virtual_address + i, flags, cache);
    }

    bool map_range(uintptr_t virtual_address, uintptr_t physical_address,
                   size_t length, int prot, int flags);

    pagemap_t* fork();

    void load();
    void save();

    pagemap_t();
    ~pagemap_t();
};

extern pagemap_t* kernel_pagemap;

bool is_canonical(uintptr_t addr);
uintptr_t flags_to_arch(size_t flags);

void init();

void destroy_pagemap(pagemap_t* pagemap);
[[gnu::weak]] void arch_init();
}  // namespace memory::paging