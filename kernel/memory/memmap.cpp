#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory/physical.hpp>
#include <memory/virtual.hpp>
#include <memory>
#include <utils/math.hpp>

namespace memory::paging {
namespace memmap {
struct global {
    std::vector<std::shared_ptr<local>> locals;
    std::unique_ptr<memory::paging::pagemap_t> shadow;

    uintptr_t base;
    size_t length;
    long offset;

    bool map_page(uintptr_t virtual_address, uintptr_t physical_address,
                  int prot);
};

struct local {
    std::shared_ptr<global> global;
    memory::paging::pagemap_t* pagemap;

    uintptr_t base;
    size_t length;
    long offset;

    int prot;
    int flags;
};

bool global::map_page(uintptr_t virtual_address, uintptr_t physical_address,
                      int prot) {
    size_t flags = memory::paging::USER;

    if (prot & protocol_read)
        flags |= memory::paging::READ;
    if (prot & protocol_write)
        flags |= memory::paging::WRITE;
    if (prot & protocol_exec)
        flags |= memory::paging::EXEC;

    if (!this->shadow->map(virtual_address, physical_address, flags))
        return false;

    for (const std::shared_ptr<local>& local : this->locals) {
        if (virtual_address < local->base ||
            virtual_address >= local->base + local->length)
            continue;
        if (!local->pagemap->map(virtual_address, physical_address, flags))
            return false;
    }

    return true;
}
}  // namespace memmap

std::optional<std::tuple<std::shared_ptr<memmap::local>, size_t, size_t>>
pagemap_t::address_to_range(uintptr_t address) {
    for (const std::shared_ptr<memmap::local>& local : this->ranges) {
        if (address < local->base || address >= local->base + local->length)
            continue;

        size_t map_page = address / this->page_size;
        size_t page = local->offset / this->page_size +
                      (map_page - local->base / this->page_size);

        return std::tuple<std::shared_ptr<memmap::local>, size_t, size_t>{
            local, map_page, page};
    }

    return std::nullopt;
}

pagemap_t::~pagemap_t() {
    while (this->ranges.size() > 0) {
        std::shared_ptr<memmap::local> local = this->ranges.at(0);
        this->unmap(local->base, local->length);
    }

    destroy_pagemap(this);
}

bool pagemap_t::map_range(uintptr_t virtual_address, uintptr_t physical_address,
                          size_t length, int prot, int flags) {
    flags |= memmap::map_anonymous;

    size_t aligned_virtual_address =
        utils::align_down(virtual_address, this->page_size);
    size_t aligned_length = utils::align_up(
        length + (virtual_address - aligned_virtual_address), this->page_size);

    std::shared_ptr<memmap::global> global = std::make_shared<memmap::global>();
    std::shared_ptr<memmap::local> local = std::make_shared<memmap::local>();

    global->shadow = std::make_unique<pagemap_t>();
    global->base = aligned_virtual_address;
    global->length = aligned_length;

    local->global = global;
    local->pagemap = this;
    local->base = aligned_virtual_address;
    local->prot = prot;
    local->flags = flags;

    global->locals.push_back(local);

    this->ranges.push_back(local);

    for (size_t i = 0; i < aligned_length; i += this->page_size)
        if (!global->map_page(aligned_virtual_address + i, physical_address + i,
                              prot))
            return false;

    return true;
}

pagemap_t* pagemap_t::fork() {
    pagemap_t* pagemap = new pagemap_t;

    for (const std::shared_ptr<memmap::local>& local : this->ranges) {
        std::shared_ptr<memmap::global> global = local->global;
        std::shared_ptr<memmap::local> new_local =
            std::make_shared<memmap::local>(*local);

        if (local->flags & memmap::map_shared) {
            global->locals.push_back(new_local);

            for (uintptr_t i = local->base; i < local->base + local->length;
                 i += this->page_size) {
                pt_entry_t* old_pt_entry =
                    this->virtual_to_pt_entry(i, false, this->page_size);
                if (old_pt_entry == nullptr)
                    continue;

                pagemap->virtual_to_pt_entry(i, true, pagemap->page_size)
                    ->value = old_pt_entry->value;
            }
        } else {
            std::shared_ptr<memmap::global> new_global =
                std::make_shared<memmap::global>();

            new_global->shadow = std::make_unique<pagemap_t>();
            new_global->base = global->base;
            new_global->length = global->length;
            new_global->offset = global->offset;
            new_global->locals.push_back(new_local);

            for (uintptr_t i = local->flags; i < local->base + local->length;
                 i += this->page_size) {
                pt_entry_t* old_pt_entry =
                    this->virtual_to_pt_entry(i, true, pagemap->page_size);
                if (old_pt_entry == nullptr)
                    continue;

                pt_entry_t* new_pt_entry =
                    this->virtual_to_pt_entry(i, true, pagemap->page_size);
                pt_entry_t* new_shadow_pt_entry =
                    new_global->shadow->virtual_to_pt_entry(
                        i, true, new_global->shadow->page_size);
                uintptr_t old_page = old_pt_entry->get_address();
                auto page = memory::physical::alloc<uintptr_t>(
                    this->page_size / memory::physical::page_size);

                memcpy(reinterpret_cast<void*>(utils::tohh(page)),
                       reinterpret_cast<void*>(utils::tohh(old_page)),
                       this->page_size);

                new_pt_entry->value = 0;
                new_pt_entry->set_flags(old_pt_entry->get_flags(), true);
                new_pt_entry->set_address(page);
                new_shadow_pt_entry->value = new_pt_entry->value;
            }
        }

        pagemap->ranges.push_back(new_local);
    }

    return pagemap;
}
}  // namespace memory::paging