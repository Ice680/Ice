set(CMAKE_CXX_STANDARD 20)

# add_library(
#     assembly
# )

add_executable(
    kernel
    kmain.cpp
)
set_target_properties(kernel PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.elf)
target_compile_definitions(kernel PRIVATE ${ARCH})
target_compile_options(
    kernel PRIVATE
    -fshort-wchar
    -ffreestanding
    -fsized-deallocation
    # -fuse-ld=lld

    -fno-pic
    -fno-pie
    -fno-rtti
    -fno-fast-math
    -fno-exceptions
    -fno-use-cxa-atexit
    -fno-stack-protector
    -fno-associative-math
    -fno-omit-frame-pointer
    
    -Wall
    -Wextra
    -Werror
    -Wno-volatile
    -Wno-macro-redefined
    -Wno-non-virtual-dtor
    -Wno-unused-parameter
    -Wno-builtin-macro-redefined
    -Wno-error-depracted-enum-enum-conversion
    
    -mno-sse
    -mno-sse2
    
    -mcmodel=kernel
    
    -target x86_64-pc-elf
    -c
)

target_link_options(
    kernel PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/linker-${ARCH}.ld
    -zmax-page-size=0x1000
    -nostdlib
    -static
)
target_include_directories(kernel PRIVATE include)
# target_link_libraries(kernel PRIVATE assembly)