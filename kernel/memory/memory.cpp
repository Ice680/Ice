#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>

#include <memory/memory.hpp>
#include <memory/physical.hpp>
#include <utils/math.hpp>

namespace memory {
std::allocator<slab_allocator> allocator;

void slab_t::init(size_t size) {
    this->size = size;
    this->head = utils::tohh(physical::alloc<uintptr_t>());

    size_t available =
        0x1000 - utils::align_up(sizeof(slab_header), this->size);
    slab_header* slab_ptr = reinterpret_cast<slab_header*>(this->head);

    slab_ptr->slab = this;
    this->head += utils::align_up(sizeof(slab_header), this->size);

    uint64_t* array = reinterpret_cast<uint64_t*>(this->head);
    size_t max = available / this->size - 1;
    size_t count = this->size / 8;

    for (size_t i = 0; i < max; i++)
        array[i * count] = reinterpret_cast<uint64_t>(&array[(i + 1) * count]);
    array[max * count] = 0;
}

void* slab_t::alloc() {
    if (this->head == 0)
        this->init(this->size);

    uint64_t* old_free = reinterpret_cast<uint64_t*>(this->head);

    this->head = old_free[0];
    memset(old_free, 0, this->size);

    return old_free;
}

void slab_t::free(void* ptr) {
    if (ptr == nullptr)
        return;

    uint64_t* new_head = static_cast<uint64_t*>(ptr);

    new_head[0] = this->head;
    this->head = reinterpret_cast<uintptr_t>(new_head);
}

slab_allocator::slab_allocator() {
    this->slabs[0].init(16);
    this->slabs[1].init(32);
    this->slabs[2].init(48);
    this->slabs[3].init(80);
    this->slabs[4].init(128);
    this->slabs[5].init(192);
    this->slabs[6].init(288);
    this->slabs[7].init(448);
    this->slabs[8].init(672);
    this->slabs[9].init(1024);
}

slab_t* slab_allocator::get_slab(size_t size) {
    for (slab_t& slab : this->slabs)
        if (slab.size >= size)
            return &slab;
    return nullptr;
}

void* slab_allocator::big_malloc(size_t size) {
    size_t pages = utils::div_roundup(size, 0x1000);
    void* ptr = utils::tohh(physical::alloc(pages + 1));
    big_alloc_meta* meta_data = reinterpret_cast<big_alloc_meta*>(ptr);

    meta_data->pages = pages;
    meta_data->size = size;

    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) + 0x1000);
}

void* slab_allocator::big_realloc(void* old_ptr, size_t size) {
    if (old_ptr == nullptr)
        return this->malloc(size);

    big_alloc_meta* meta_data = reinterpret_cast<big_alloc_meta*>(
        reinterpret_cast<uintptr_t>(old_ptr) - 0x1000);
    size_t old_size = meta_data->size;

    if (utils::div_roundup(old_size, 0x1000) ==
        utils::div_roundup(size, 0x1000)) {
        meta_data->size = size;
        return nullptr;
    }

    if (size == 0) {
        this->free(old_ptr);
        return nullptr;
    }

    if (size < old_size)
        old_size = size;

    void* new_ptr = this->malloc(size);
    if (new_ptr == nullptr)
        return old_ptr;

    memcpy(new_ptr, old_ptr, old_size);
    this->free(old_ptr);

    return new_ptr;
}

void slab_allocator::big_free(void* ptr) {
    big_alloc_meta* meta_data = reinterpret_cast<big_alloc_meta*>(
        reinterpret_cast<uintptr_t>(ptr) - 0x1000);
    physical::free(utils::fromhh(meta_data), meta_data->pages + 1);
}

size_t slab_allocator::big_alloc_size(void* ptr) {
    big_alloc_meta* meta_data = reinterpret_cast<big_alloc_meta*>(
        reinterpret_cast<uintptr_t>(ptr) - 0x1000);
    return meta_data->size;
}

void* slab_allocator::malloc(size_t size) {
    slab_t* slab = this->get_slab(size);
    if (slab == nullptr)
        return this->big_malloc(size);
    return slab->alloc();
}

void* slab_allocator::calloc(size_t num, size_t size) {
    void* ptr = this->malloc(num * size);
    if (ptr == nullptr)
        return nullptr;

    memset(ptr, 0, num * size);
    return ptr;
}

void* slab_allocator::realloc(void* old_ptr, size_t size) {
    if (old_ptr == nullptr)
        return this->malloc(size);

    if ((reinterpret_cast<uintptr_t>(old_ptr) & 0xFFF) == 0)
        return this->big_realloc(old_ptr, size);

    slab_t* slab = reinterpret_cast<slab_header*>(
                       reinterpret_cast<uintptr_t>(old_ptr) & ~0xFFF)
                       ->slab;
    size_t old_size = slab->size;

    if (size == 0) {
        this->free(old_ptr);
        return nullptr;
    }

    if (size < old_size)
        old_size = size;

    void* new_ptr = this->malloc(size);

    if (new_ptr == nullptr)
        return old_ptr;

    memcpy(new_ptr, old_ptr, old_size);
    this->free(old_ptr);

    return new_ptr;
}

void slab_allocator::free(void* ptr) {
    if (ptr == nullptr)
        return;

    if ((reinterpret_cast<uintptr_t>(ptr) & 0xFFF) == 0)
        return this->big_free(ptr);
    slab_t* slab = reinterpret_cast<slab_header*>(
                       reinterpret_cast<uintptr_t>(ptr) & ~0xFFF)
                       ->slab;
    slab->free(ptr);
}

size_t slab_allocator::alloc_size(void* ptr) {
    if (ptr == nullptr)
        return 0;

    if ((reinterpret_cast<uintptr_t>(ptr) & 0xFFF) == 0)
        return this->big_alloc_size(ptr);
    slab_t* slab = reinterpret_cast<slab_header*>(
                       reinterpret_cast<uintptr_t>(ptr) & ~0xFFF)
                       ->slab;
    return slab->size;
}
}  // namespace memory

void* operator new(size_t size) {
    return malloc(size);
}

void* operator new(size_t size, std::align_val_t) {
    return malloc(size);
}

void* operator new[](size_t size) {
    return malloc(size);
}

void* operator new[](size_t size, std::align_val_t) {
    return malloc(size);
}

void operator delete(void* ptr) noexcept {
    free(ptr);
}

void operator delete(void* ptr, std::align_val_t) noexcept {
    free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
    free(ptr);
}

void operator delete[](void* ptr) noexcept {
    free(ptr);
}

void operator delete[](void* ptr, std::align_val_t) noexcept {
    free(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
    free(ptr);
}