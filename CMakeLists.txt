cmake_minimum_required(VERSION 3.12)

# TODO: better project name
project(Ice VERSION 0.0.1 LANGUAGES C CXX ASM)

include(cmake/config.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KERNEL_DIR ${CMAKE_BINARY_DIR}/kernel/)
set(SCRIPTS_DIR ${SOURCE_DIR}/scripts)
set(ISO_ROOT ${SOURCE_DIR}/iso_root)

set(ISO_PATH "${SOURCE_DIR}/${PROJECT_NAME}.iso")

add_subdirectory(kernel)

find_program(XORRISO xorriso)
if(XORRISO)
    message(VERBOSE "Found xorriso, creating target: image_iso")    
    
    set(
        XORRISO_FLAGS 
        -as mkisofs 
        -b limine-cd.bin
        -no-emul-boot
        -boot-load-size 4
        -boot-info-table 
        --efi-boot limine-cd-efi.bin 
        -efi-boot-part 
        --efi-boot-image 
        --protective-msdos-label
    )
    
    add_custom_target(
        image_iso
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_ROOT}
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_DIR}/${PROJECT_NAME}.elf ${ISO_ROOT}
        COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_DIR}/boot/limine.sys ${ISO_ROOT}
        COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_DIR}/boot/limine-cd.bin ${ISO_ROOT}
        COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_DIR}/boot/limine-cd-efi.bin ${ISO_ROOT}
        COMMAND ${CMAKE_COMMAND} -E copy ${SCRIPTS_DIR}/boot/limine.cfg ${ISO_ROOT}
        COMMAND ${XORRISO} ${XORRISO_FLAGS} ${ISO_ROOT} -o ${ISO_PATH}
        COMMAND make -C ${SOURCE_DIR}/boot
        COMMAND ${SOURCE_DIR}/boot/limine-deploy ${ISO_PATH}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${ISO_ROOT}
        COMMENT "Generating El-Torito ISO-9660 boot media"
        WORKING_DIRECTORY ${SOURCE_DIR}
        USES_TERMINAL
    )
    
    add_dependencies(image_iso kernel)
else()
    message(NOTICE "[31;5mWARN: MISSING PROGRAM!  Could not find `xorriso`, image_iso target has not been generated. See dependencies in README.[m")
endif()

find_program(QEMU_PROGRAM qemu-system-${ARCH})
if(QEMU_PROGRAM)
    message(VERBOSE "Found QEMU for ${ARCH}, creating targets: run_qemu")
    set(
        QEMU_FLAGS
        -machine q35
        # cpu: Broadwell, Cascadelake-Server, Cooperlake, Conroe, core2duo,
        #      Denverton, Dhyana, EPYC, Haswell, IvyBridge, kvm64, max,
        #      Nehalem, Penryn, qemu64, SandyBridge, Skylake-[Client|Server],
        #      Snowridge, Westmere
        # -cpu qemu64
        # Multiple processors (commented until utilized).
        #-smp 8,sockets=2,cores=2,threads=2,maxcpus=8
        # One hundred megabytes of RAM.
        -m 2G
        # Use local time as real time clock base 
        # -rtc base=localtime,clock=host,driftfix=none
        # No networking drivers yet.
        # -net none
        # Show extra information regarding triple faults
        # -d cpu_reset
        # Use stdio as serial input and output
        # This allows debug messages to reach the terminal
        -serial stdio
        # -display gtk,gl=off
        -boot d
    )
    
    # if(CMAKE_HOST_WIN32)
    #     list(
    #         APPEND QEMU_FLAGS
    #         -audiodev dsound,id=audio_device
    #         -machine pcspk-audiodev=audio_device
    #     )
    # else()
    #     list(
    #         APPEND QEMU_FLAGS
    #         -audiodev oss,id=audio_device
    #         -machine pcspk-audiodev=audio_device
    #     )
    # endif()

    if(QEMU_DEBUG)
        list(APPEND QEMU_FLAGS -S -s)
    endif()

    set(OVMF "${SOURCE_DIR}/ovmf/OVMF.fd")
    # set(OVMF_VARS "${SOURCE_DIR}/ovmf/OVMF_VARS-pure-efi.fd")

    list(
        APPEND QEMU_FLAGS
        -bios ${OVMF}
    )

    add_custom_target(
        run_qemu
        COMMAND ${QEMU_PROGRAM} ${QEMU_FLAGS} -cdrom ${ISO_PATH}
        COMMENT "Running QEMU for ${ARCH} from ISO `${PROJECT_NAME}.iso`"
        USES_TERMINAL
        WORKING_DIRECTORY ${SOURCE_DIR}
        VERBATIM
    )
    add_dependencies(run_qemu kernel image_iso)
else()
    message("QEMU for ${ARCH} not found on our system, skipping run_qemu target generation.")
endif()