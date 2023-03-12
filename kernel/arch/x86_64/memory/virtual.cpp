#include <arch/x86_64/cpu/cpu.hpp>
#include <cstddef>
#include <cstdint>
#include <memory/physical.hpp>
#include <memory/virtual.hpp>
#include <utils/math.hpp>
#include <sys/logger.hpp>

namespace memory::paging {
enum {
    present = 1,
    write = 2,
    user_super = 4,
    write_through = 8,
    cache_disable = 16,
    accessed = 32,
    larger_pages = 128,
    pat4k = 128,
    global = 256,
    custom0 = 512,
    custom1 = 1024,
    custom2 = 2048,
    patlg = 4096,
    no_exec = (1ul << 63)
};

struct [[gnu::packed]] page_table_t {
    pt_entry_t entries[512];
};

static bool gib1_pages = false;

uintptr_t pa_mask = 0x000FFFFFFFFFF000;

static page_table_t* get_next_lvl(page_table_t* curr_lvl, size_t entry,
                                  bool allocate = true) {
    if (curr_lvl == nullptr)
        return nullptr;

    page_table_t* ret = nullptr;

    if (curr_lvl->entries[entry].get_flags(present)) {
        ret = reinterpret_cast<page_table_t*>(
            utils::tohh(curr_lvl->entries[entry].get_address()));
    } else if (allocate == true) {
        ret = new page_table_t;

        curr_lvl->entries[entry].set_address(
            utils::fromhh(reinterpret_cast<uint64_t>(ret)));
        curr_lvl->entries[entry].set_flags(present | write | user_super, true);
    }

    return ret;
}

pt_entry_t* pagemap_t::virtual_to_pt_entry(uint64_t virtual_address,
                                           bool allocate, uint64_t page_size) {
    size_t pml4_entry = (virtual_address & (0x1FFULL << 39)) >> 39;
    size_t pml3_entry = (virtual_address & (0x1FFULL << 30)) >> 30;
    size_t pml2_entry = (virtual_address & (0x1FFULL << 21)) >> 21;
    size_t pml1_entry = (virtual_address & (0x1FFULL << 12)) >> 12;

    page_table_t *pml4, *pml3, *pml2, *pml1;

    pml4 = this->top_level;
    if (pml4 == nullptr)
        return nullptr;

    pml3 = get_next_lvl(pml4, pml4_entry, allocate);
    if (pml3 == nullptr)
        return nullptr;

    if (page_size == this->llpage_size)
        return &pml3->entries[pml3_entry];

    pml2 = get_next_lvl(pml3, pml3_entry, allocate);
    if (pml2 == nullptr)
        return nullptr;

    if (page_size == this->lpage_size)
        return &pml2->entries[pml2_entry];

    pml1 = get_next_lvl(pml2, pml2_entry, allocate);
    if (pml1 == nullptr)
        return nullptr;

    return &pml1->entries[pml1_entry];
}

/*
* Uncachable:     PAT0:  PAT = 0, PCD = 0, PWT = 0
* WriteCombining: PAT1:  PAT = 0, PCD = 0, PWT = 1
* None            PAT2:  PAT = 0, PCD = 1, PWT = 0
* None            PAT3:  PAT = 0, PCD = 1, PWT = 1
* WriteThrough:   PAT4:  PAT = 1, PCD = 0, PWT = 0
* WriteProtected: PAT5:  PAT = 1, PCD = 0, PWT = 1
* WriteBack:      PAT6:  PAT = 1, PCD = 1, PWT = 0
* Uncached:       PAT7:  PAT = 1, PCD = 1, PWT = 1
*/
static uint64_t cache_to_flags(caching cache, bool larger_pages) {
    uint64_t pat_bit = (larger_pages ? patlg : pat4k);
    uint64_t ret = 0;

    switch (cache) {
        case UNCACHABLE:
            break;
        case WRITE_COMBINING:
            ret |= write_through;
            break;
        case WRITE_THROUGH:
            ret |= pat_bit;
            break;
        case WRITE_PROTECTED:
            ret |= pat_bit | write_through;
            break;
        case WRITE_BACK:
            ret |= pat_bit | cache_disable;
            break;
        default:
            break;
    }

    return ret;
}

uintptr_t pagemap_t::virtual_to_physical(uintptr_t virtual_address,
                                         size_t flags) {
    size_t page_size = this->get_page_size(flags);
    pt_entry_t* pml_entry =
        this->virtual_to_pt_entry(virtual_address, false, page_size);

    if (pml_entry == nullptr || !pml_entry->get_flags(present))
        return invalid_addr;

    return pml_entry->get_address() + (virtual_address % page_size);
}

bool pagemap_t::map(uintptr_t virtual_address, uintptr_t physical_address,
                    size_t flags, caching cache) {
    auto map_one = [this](uintptr_t virtual_address, uintptr_t physical_address,
                          size_t flags, caching cache, size_t page_size) {
        pt_entry_t* pml_entry =
            this->virtual_to_pt_entry(virtual_address, true, page_size);

        if (pml_entry == nullptr) {
            logger::log_error(
                "Virtual Memory Manager couldn't get page map entry for "
                "address 0x%x",
                virtual_address);
            return false;
        }

        size_t real_flags = flags_to_arch(flags) |
                            cache_to_flags(cache, page_size != this->page_size);

        pml_entry->set_address(physical_address);
        pml_entry->set_flags(real_flags, true);

        return true;
    };

    size_t page_size = this->get_page_size(flags);

    if (page_size == this->llpage_size && gib1_pages == false) {
        flags &= ~LLPAGE;
        flags |= LPAGE;

        for (size_t i = 0; i < GiB1; i += MiB2)
            if (!map_one(virtual_address + i, physical_address + i, flags,
                         cache, MiB2))
                return false;
        return true;
    }

    return map_one(virtual_address, physical_address, flags, cache, page_size);
}

bool pagemap_t::unmap(uintptr_t virtual_address, size_t flags) {
    auto unmap_one = [this](uintptr_t virtual_address, size_t flags) {
        pt_entry_t* pml_entry =
            this->virtual_to_pt_entry(virtual_address, false, page_size);

        if (pml_entry == nullptr) {
            logger::log_error(
                "Virtual Memory Manager couldn't get page map entry for "
                "address 0x%x",
                virtual_address);
            return false;
        }

        pml_entry->value = 0;
        system::cpu::invlpg(virtual_address);

        return true;
    };

    size_t page_size = this->get_page_size(flags);

    if (page_size == this->llpage_size && gib1_pages == false) {
        flags &= ~LLPAGE;
        flags |= LPAGE;

        for (size_t i = 0; i < GiB1; i += MiB2)
            if (!unmap_one(virtual_address + i, MiB2))
                return false;
        return true;
    }

    return unmap_one(virtual_address, page_size);
}

bool pagemap_t::set_flags(uintptr_t virtual_address, size_t flags,
                          caching cache) {
    size_t page_size = this->get_page_size(flags);
    pt_entry_t* pml_entry =
        this->virtual_to_pt_entry(virtual_address, true, page_size);

    if (pml_entry == nullptr) {
        logger::log_error(
            "Virtual Memory Manager couldn't get page map entry for "
            "address 0x%x",
            virtual_address);
        return false;
    }

    size_t real_flags = flags_to_arch(flags) |
                        cache_to_flags(cache, page_size != this->page_size);
    uintptr_t address = pml_entry->get_address();

    pml_entry->value = 0;
    pml_entry->set_address(address);
    pml_entry->set_flags(real_flags, true);

    return true;
}

void pagemap_t::load() {
    wrreg(cr3, utils::fromhh(reinterpret_cast<uint64_t>(this->top_level)));
}

void pagemap_t::save() {
    this->top_level = reinterpret_cast<page_table_t*>(utils::tohh(rdreg(cr3)));
}

pagemap_t::pagemap_t() : top_level(new page_table_t) {
    this->llpage_size = GiB1;
    this->lpage_size = MiB2;
    this->page_size = KiB4;

    if (kernel_pagemap == nullptr) {
        for (size_t i = 256; i < 512; i++)
            get_next_lvl(this->top_level, i, true);
    } else {
        for (size_t i = 256; i < 512; i++)
            this->top_level->entries[i] = kernel_pagemap->top_level->entries[i];
    }
}

bool is_canonical(uintptr_t address) {
    return (address <= 0x00007FFFFFFFFFFFULL) ||
           (address >= 0xFFFF800000000000ULL);
}

uintptr_t flags_to_arch(size_t flags) {
    uintptr_t ret = present;

    if (flags & WRITE)
        ret |= write;
    if (flags & USER)
        ret |= user_super;
    if (!(flags & EXEC))
        ret |= no_exec;
    if (flags & GLOBAL)
        ret |= global;
    if (is_lpage(flags))
        ret |= larger_pages;

    return ret;
}

static void destroy_level(page_table_t* pml, size_t start, size_t end,
                          size_t level) {
    if (level == 0)
        return;

    for (size_t i = start; i < end; i++) {
        page_table_t* next = get_next_lvl(pml, i, false);

        if (next == nullptr)
            continue;

        destroy_level(next, 0, 512, level - 1);
    }

    delete pml;
}

void destroy_pagemap(pagemap_t* pagemap) {
    destroy_level(pagemap->top_level, 0, 256, 4);
}

void arch_init() {
    uint32_t a, b, c, d;
    gib1_pages = system::cpu::id(0x80000001, 0, a, b, c, d) &&
                 ((d & 1 << 26) == 1 << 26);
}
}  // namespace memory::paging