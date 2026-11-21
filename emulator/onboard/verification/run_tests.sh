#!/usr/bin/env bash
# Build the standalone backing test and both combined IOU/MMU bus variants.
set -euo pipefail

tests_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "$tests_dir/../../.." && pwd)"
build_dir="$(mktemp -d)"
trap 'rm -rf -- "$build_dir"' EXIT

CC="${CC:-cc}"
export VERILATOR_ROOT="${VERILATOR_ROOT:-$HOME/Tools/verilator}"
verilator="$VERILATOR_ROOT/bin/verilator"

onboard="$repo_dir/emulator/onboard"
cpld="$repo_dir/cpld"
headers=("-I$onboard" "-I$cpld" "-I$repo_dir/system"
         "-I$repo_dir/emulator/rtl_video_ctrl" "-I$repo_dir/emulator/slot_peripherals")

"$CC" -std=c11 -Wall -Wextra -Werror -O2 -I"$onboard" \
    "$tests_dir/test_rom_backing.c" "$onboard/rom_backing.c" -o "$build_dir/test_rom_backing"
"$build_dir/test_rom_backing" "$build_dir/backing.bin"

"$CC" -std=c11 -Wall -Wextra -Werror -O2 "${headers[@]}" \
    -c "$onboard/memory_controller.c" -o "$build_dir/memory_controller.o"
"$CC" -std=c11 -Wall -Wextra -Werror -O2 -I"$onboard" \
    -c "$onboard/rom_backing.c" -o "$build_dir/rom_backing.o"

run_bus_test() {
    local tag="$1"
    shift
    local objdir="$build_dir/$tag"

    "$verilator" --cc --exe --build -j 2 -Wall --pins-inout-enables \
        -D_DEBUG_GPIO -DEXTERNAL_CPU -DVERILATOR_RUN -DEXTERNAL_MEMORY -DIOU_COMPLETE "$@" \
        --top-module IOU_MMU_tb --Mdir "$objdir" \
        "-I$repo_dir/rtl_IOU" "-I$repo_dir/rtl_MMU" \
        -CFLAGS "${headers[*]} $*" \
        -LDFLAGS "$build_dir/memory_controller.o $build_dir/rom_backing.o" \
        "$repo_dir/rtl_IOU_MMU_TB/IOU_MMU_tb.v" "$tests_dir/test_rom_bus.cpp" \
        "$cpld/rtl_model.cpp" "$cpld/rtl_interface.cpp" "$cpld/rtl_cpu_bus.cpp" \
        "$cpld/rtl_memory_bus.cpp" "$cpld/rtl_interconnect.cpp" "$cpld/rtl_spi_slave.cpp"

    printf 'Combined ROM bus (%s)\n' "$tag"
    "$objdir/VIOU_MMU_tb" "$build_dir/bus-$tag.bin"
}

run_bus_test default
run_bus_test full -DFULL_MMU_ADDR
