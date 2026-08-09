#!/usr/bin/env bash
#
# Flash the built firmware via OpenOCD + ST-Link (SWD).
# Usage: ./Scripts/flash.sh   (or: cmake --build build --target flash)
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Detect the CMake project name from CMakeLists.txt.
PROJECT_NAME="$(sed -n 's/^project(\([^ (]*\).*/\1/p' "${PROJECT_DIR}/CMakeLists.txt" | head -n 1)"
if [[ -z "${PROJECT_NAME}" ]]; then
    echo "Error: cannot detect project name in ${PROJECT_DIR}/CMakeLists.txt" >&2
    exit 1
fi

# Locate the ELF (plain build/ or preset build dirs).
ELF=""
for cand in "${PROJECT_DIR}/build/${PROJECT_NAME}" \
            "${PROJECT_DIR}/build/debug/${PROJECT_NAME}" \
            "${PROJECT_DIR}/build/release/${PROJECT_NAME}"; do
    if [[ -f "${cand}" ]]; then
        ELF="${cand}"
        break
    fi
done

if [[ ! -f "${ELF}" ]]; then
    echo "Firmware not found: ${ELF}" >&2
    echo "Build it first:  cmake --build build" >&2
    exit 1
fi

exec openocd \
    -f interface/stlink.cfg \
    -c "transport select hdlc_swd" \
    -f target/stm32f1x.cfg \
    -c "program ${ELF} verify reset exit"
