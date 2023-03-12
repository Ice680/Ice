#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include <kernel.hpp>
#include <memory/physical.hpp>
#include <memory/virtual.hpp>
#include <sys/logger.hpp>
#include <sys/panic.hpp>
#include <utils/math.hpp>

namespace memory::paging {
pagemap_t* kernel_pagemap = nullptr;

void init() {
    logger::log_info("Initializing Virtual Memory Management");

    limine_memmap_entry** memmaps = memmap_request.response->entries;
    size_t memmap_count = memmap_request.response->entry_count;

    arch_init();

    kernel_pagemap = new pagemap_t();

    auto [page_size, flags] = kernel_pagemap->required_size(GiB1 * 4);

    for (size_t i = 0; i < GiB1 * 4; i += page_size) {
        assert(kernel_pagemap->map(i, i, RWX | flags));
        assert(kernel_pagemap->map(utils::tohh(i), i, RW | flags));
    }

    for (size_t i = 0; i < memmap_count; i++) {
        limine_memmap_entry* memmap = memmaps[i];

        uint64_t base =
            utils::align_down(memmap->base, kernel_pagemap->page_size);
        uint64_t top = utils::align_up(memmap->base + memmap->length,
                                       kernel_pagemap->page_size);

        if (top < GiB1 * 4)
            continue;

        caching cache = default_caching;

        if (memmap->type == LIMINE_MEMMAP_FRAMEBUFFER)
            cache = FRAMEBUFFER;

        for (uint64_t t = base; t < top; t += kernel_pagemap->page_size) {
            if (t < GiB1 * 4)
                continue;

            assert(kernel_pagemap->map(t, t, RWX, cache));
            assert(kernel_pagemap->map(utils::tohh(t), t, RW, cache));
        }
    }

    for (size_t i = 0; i < kernel_file_request.response->kernel_file->size;
         i += kernel_pagemap->page_size) {
        uint64_t physical_addr =
            kernel_address_request.response->physical_base + i;
        uint64_t virtual_addr =
            kernel_address_request.response->virtual_base + i;
        assert(kernel_pagemap->map(virtual_addr, physical_addr, RWX));
    }

    kernel_pagemap->load();
}
}  // namespace memory::paging