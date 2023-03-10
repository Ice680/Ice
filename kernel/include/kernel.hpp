#pragma once

#include <limine.h>
#include <cstddef>
#include <cstdint>

extern uintptr_t hhdm_offset;

extern volatile limine_hhdm_request hhdm_request;
extern volatile limine_terminal_request terminal_request;
extern volatile limine_memmap_request memmap_request;
extern volatile limine_kernel_file_request kernel_file_request;
extern volatile limine_kernel_address_request kernel_address_request;