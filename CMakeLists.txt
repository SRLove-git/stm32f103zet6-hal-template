cmake_minimum_required(VERSION 3.22)

project(stm32f103zet6-hal-template
        VERSION 1.0.0
        DESCRIPTION "HAL project template for ALIENTEK Elite STM32F103ZET6"
        LANGUAGES C ASM)

if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR
        "Please configure with the ARM toolchain file, e.g.:\n"
        "  cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug/Release/MinSizeRel)" FORCE)
endif()

# Use newlib-nano instead of the full newlib C library.
# - OFF (default): full newlib, printf() supports %f out of the box.
# - ON: smaller libc; float printf is kept by forcing _printf_float in.
option(USE_NEWLIB_NANO "Use newlib-nano (smaller binary, keeps %f support)" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# -----------------------------------------------------------------------------
# Chip / toolchain settings
# -----------------------------------------------------------------------------
set(MCU_FLAGS -mcpu=cortex-m3 -mthumb -mfloat-abi=soft)
add_compile_options(${MCU_FLAGS} -ffunction-sections -fdata-sections)

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/Linker/STM32F103ZETX_FLASH.ld")

# -----------------------------------------------------------------------------
# Sources
#
# Core/Src and BSP/Src are picked up automatically (CONFIGURE_DEPENDS re-globs
# on every build), so adding a new .c file requires no CMake edits.
# -----------------------------------------------------------------------------
file(GLOB HAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal*.c"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_*.c")
list(FILTER HAL_SOURCES EXCLUDE REGEX "_template\\.c$") # msp/timebase templates

# Third-party ST sources: silence benign warnings triggered by the strict
# project-wide warning set (strict warnings still apply to Core/BSP code).
set_source_files_properties(${HAL_SOURCES} PROPERTIES
    COMPILE_OPTIONS "-Wno-cast-align;-Wno-unused-parameter")

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Core/Src/*.c")
file(GLOB BSP_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/BSP/Src/*.c")

add_executable(${PROJECT_NAME}
    ${CORE_SOURCES}
    ${BSP_SOURCES}
    ${HAL_SOURCES}
    "${CMAKE_SOURCE_DIR}/Startup/startup_stm32f103xe.s")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    STM32F103xE      # device family: STM32F103ZE (512 KB flash / 64 KB RAM)
    USE_HAL_DRIVER)  # enable the HAL layer

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/Core/Inc"
    "${CMAKE_SOURCE_DIR}/BSP/Inc"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc")

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wshadow -Wundef -Wwrite-strings -Wcast-align)

target_link_options(${PROJECT_NAME} PRIVATE
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,--no-warn-rwx-segments
    "-Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map"
    "-T${LINKER_SCRIPT}")

if(USE_NEWLIB_NANO)
    # Keep %f/%e/%g working under newlib-nano. The symbol must be passed to
    # the linker as separate -u / _printf_float arguments (a single
    # "-u _printf_float" token would be parsed as symbol " _printf_float").
    target_link_options(${PROJECT_NAME} PRIVATE
        --specs=nano.specs
        -Wl,-u,_printf_float)
endif()

# -----------------------------------------------------------------------------
# Post-build artifacts: .hex / .bin + size report
# -----------------------------------------------------------------------------
find_program(SIZE_TOOL arm-none-eabi-size)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   "$<TARGET_FILE:${PROJECT_NAME}>" "${PROJECT_NAME}.hex"
    COMMAND ${CMAKE_OBJCOPY} -O binary "$<TARGET_FILE:${PROJECT_NAME}>" "${PROJECT_NAME}.bin"
    COMMAND ${SIZE_TOOL} "$<TARGET_FILE:${PROJECT_NAME}>"
    COMMENT "Generating HEX/BIN artifacts"
    VERBATIM)

# -----------------------------------------------------------------------------
# Flash / erase targets (OpenOCD + ST-Link, SWD)
# -----------------------------------------------------------------------------
find_program(OPENOCD openocd)
if(OPENOCD)
    set(STM32_DEBUG_PROBE "stlink" CACHE STRING
        "OpenOCD debug probe: stlink (default) or cmsis-dap")
    set_property(CACHE STM32_DEBUG_PROBE PROPERTY STRINGS stlink cmsis-dap)

    if(STM32_DEBUG_PROBE STREQUAL "stlink")
        set(OPENOCD_PROBE_ARGS -f interface/stlink.cfg
                               -c "transport select hla_swd")
    elseif(STM32_DEBUG_PROBE STREQUAL "cmsis-dap")
        set(OPENOCD_PROBE_ARGS -f interface/cmsis-dap.cfg)
    else()
        message(FATAL_ERROR
            "Unsupported STM32_DEBUG_PROBE '${STM32_DEBUG_PROBE}' "
            "(use stlink or cmsis-dap)")
    endif()

    add_custom_target(flash
        COMMAND ${OPENOCD} ${OPENOCD_PROBE_ARGS} -f target/stm32f1x.cfg
                -c "program $<TARGET_FILE:${PROJECT_NAME}> verify reset exit"
        DEPENDS ${PROJECT_NAME}
        COMMENT "Flashing ${PROJECT_NAME}.elf via OpenOCD (${STM32_DEBUG_PROBE})"
        VERBATIM)
    add_custom_target(erase
        COMMAND ${OPENOCD} ${OPENOCD_PROBE_ARGS} -f target/stm32f1x.cfg
                -c "init; halt; stm32f1x mass_erase 0; exit"
        COMMENT "Mass-erasing STM32F103 flash via OpenOCD"
        VERBATIM)
else()
    message(WARNING "openocd not found in PATH - 'flash'/'erase' targets are unavailable")
endif()
