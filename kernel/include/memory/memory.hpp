#pragma once

#include <allocator>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

namespace memory {
struct slab_t {
    uintptr_t head;
    size_t size;

    void init(size_t size);
    void* alloc();
    void free(void* ptr);
};

struct slab_header {
    slab_t* slab;
};

class slab_allocator {
   public:
    bool initialized = false;
    slab_t slabs[10];

    slab_allocator();

    void* malloc(size_t size);
    void* calloc(size_t num, size_t size);
    void* realloc(void* old_ptr, size_t size);
    void free(void* ptr);
    size_t alloc_size(void* ptr);

    template <typename T = void*>
    T malloc(size_t size) {
        return reinterpret_cast<T>(this->malloc(size));
    }

    template <typename T = void*>
    T calloc(size_t num, size_t size) {
        return reinterpret_cast<T>(this->calloc(num, size));
    }

    template <typename T>
    T realloc(T oldptr, size_t size) {
        return reinterpret_cast<T>(
            this->realloc(reinterpret_cast<void*>(oldptr), size));
    }

    void free(auto ptr) { this->free(reinterpret_cast<void*>(ptr)); }

    size_t alloc_size(auto ptr) {
        return this->alloc_size(reinterpret_cast<void*>(ptr));
    }

   private:
    struct big_alloc_meta {
        size_t pages;
        size_t size;
    };

    slab_t* get_slab(size_t size);

    void* big_malloc(size_t size);
    void* big_realloc(void* old_ptr, size_t size);
    void big_free(void* ptr);
    size_t big_alloc_size(void* ptr);
};

extern std::allocator<slab_allocator> allocator;
}  // namespace memory