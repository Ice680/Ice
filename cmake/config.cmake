# System architecture
set(ARCH "x86_64" CACHE STRING "The CPU architecture that Ice will run on.")
set_property(CACHE ARCH PROPERTY STRINGS "x86_64")

# Run environment
set(MACHINE "QEMU" CACHE STRING "The machine type that Ice will run on.")
set_property(CACHE MACHINE PROPERTY STRINGS "PC" "QEMU")

# Debug?
option(QEMU_DEBUG "Start QEMU with `-S -s` flags, halting startup until a debugger has been attached." OFF)