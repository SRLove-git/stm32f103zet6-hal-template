#!/usr/bin/env bash
#
# Flash the built firmware via OpenOCD + ST-Link (SWD).
# Usage: ./Scripts/flash.sh   (or: cmake --build build --target flash)
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ELF="${SCRIPT_DIR}/../build/stm32f103zet6-hal-template"

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
