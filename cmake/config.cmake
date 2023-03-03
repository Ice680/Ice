# System architecture
set(ARCH "x86_64" CACHE STRING "The CPU architecture that Ice will run on.")
set_property(CACHE ARCH PROPERTY STRINGS "x86_64")

# Run environment
set(MACHINE "QEMU" CACHE STRING "The machine type that Ice will run on.")
set_property(CACHE MACHINE PROPERTY STRINGS "PC" "QEMU")

# Debug?
option(QEMU_DEBUG "Start QEMU with `-S -s` flags, halting startup until a debugger has been attached." OFF)

# Set linker
set(CMAKE_LINKER "ld.lld" CACHE STRING "Linker to use while linking object files")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")