# -----------------------------------------------------------------------------
# ARM bare-metal toolchain file for STM32F103 (Cortex-M3)
#
# Usage:
#   cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake
# -----------------------------------------------------------------------------

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m3)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# The target has no host OS; skip link tests during configuration.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# CPU flags shared by C and assembly, plus final link.
set(MCU_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft")

set(CMAKE_C_FLAGS_INIT            "${MCU_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_ASM_FLAGS_INIT          "${MCU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT   "${MCU_FLAGS}")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON) # gnu11

# Export compile commands for clangd / IDE usage (written into the build dir).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
