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

verify_image() {
    local image="$1"

    # Slot 0 starts with zero bytes. The entire upper bank must stay blank.
    if [[ $(wc -c < "$image") -ne 131072 ]] ||
       ! cmp -s -n 4 "$image" /dev/zero ||
       ! cmp -s -i 65536:0 -n 65536 "$image" /dev/zero; then
        echo "FAIL: wrong image size, first bytes, or reserved bank in $image" >&2
        exit 1
    fi
    sha256sum "$image"
}

"$CC" -std=c11 -Wall -Wextra -Werror -O2 -I"$onboard" \
    "$tests_dir/test_rom_backing.c" "$onboard/rom_backing.c" -o "$build_dir/test_rom_backing"
mkdir "$build_dir/blocked"
"$build_dir/test_rom_backing" "$build_dir/backing.bin" "$build_dir/blocked"
verify_image "$build_dir/backing.bin"
test -d "$build_dir/blocked"
if compgen -G "$build_dir/*.tmp.*" > /dev/null; then
    echo "FAIL: export left a temporary file" >&2
    exit 1
fi

printf '%s\n' '#define SLOT1_ROM' 'static const unsigned char slot1_rom_bin[] = {0};' > "$build_dir/short_slot.h"
if "$CC" -std=c11 -I"$onboard" -include "$build_dir/short_slot.h" \
        -c "$onboard/rom_backing.c" -o "$build_dir/short_slot.o" 2> "$build_dir/short_slot.log" ||
   [[ $(<"$build_dir/short_slot.log") != *'slot1_rom_bin must contain 4096 bytes'* ]]; then
    echo 'FAIL: a short SLOT1_ROM override must fail the size assertion' >&2
    cat "$build_dir/short_slot.log" >&2
    exit 1
fi
echo 'Short SLOT1_ROM override rejected by the size assertion'

"$CC" -std=c11 -Wall -Wextra -Werror -O2 "${headers[@]}" \
    -c "$onboard/memory_controller.c" -o "$build_dir/memory_controller.o"
"$CC" -std=c11 -Wall -Wextra -Werror -O2 -I"$onboard" \
    -include "$tests_dir/slot_rom_fixtures.h" \
    -c "$onboard/rom_backing.c" -o "$build_dir/rom_backing.o"

run_bus_test() {
    local tag="$1"
    shift
    local objdir="$build_dir/$tag"

    "$verilator" --cc --exe --build -j 2 -Wall --pins-inout-enables \
        -D_DEBUG_GPIO -DEXTERNAL_CPU -DVERILATOR_RUN -DEXTERNAL_MEMORY -DSPK_IN_MMU "$@" \
        --top-module IOU_MMU_tb --Mdir "$objdir" \
        "-I$repo_dir/rtl_IOU" "-I$repo_dir/rtl_MMU" \
        -CFLAGS "${headers[*]} $*" \
        -LDFLAGS "$build_dir/memory_controller.o $build_dir/rom_backing.o" \
        "$repo_dir/rtl_IOU_MMU_TB/IOU_MMU_tb.v" "$tests_dir/test_rom_bus.cpp" \
        "$cpld/rtl_model.cpp" "$cpld/rtl_interface.cpp" "$cpld/rtl_cpu_bus.cpp" \
        "$cpld/rtl_memory_bus.cpp" "$cpld/rtl_interconnect.cpp" "$cpld/rtl_spi_slave.cpp"

    printf 'Combined ROM bus (%s)\n' "$tag"
    "$objdir/VIOU_MMU_tb" "$build_dir/bus-$tag.bin"
    verify_image "$build_dir/bus-$tag.bin"
}

run_bus_test default
run_bus_test full -DFULL_MMU_ADDR
cmp "$build_dir/bus-default.bin" "$build_dir/bus-full.bin"
