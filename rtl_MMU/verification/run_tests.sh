#!/usr/bin/env bash
# CPLD-ROM-0001 — build and run the MMU_addr regression in both address
# configurations (default 5-bit upper, and FULL_MMU_ADDR 17-bit).
#
# Requires Verilator (expected at $HOME/Tools/verilator). Override with
# VERILATOR_ROOT.
set -euo pipefail

cd "$(dirname "$0")"

export VERILATOR_ROOT="${VERILATOR_ROOT:-$HOME/Tools/verilator}"
VERILATOR="$VERILATOR_ROOT/bin/verilator"

RTL_SRC="-I../ ../MMU_addr.v ../MMU_mem_map.v ../MMU_addr16.v ../MMU_mem_select.v ../MMU_slot_ctrl.v"
WARN="-Wall -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM -Wno-DECLFILENAME"

run_variant() {
    local tag="$1"; shift
    local defs="$1"; shift
    local objdir="obj_${tag}"
    echo "=============================================================="
    echo " Building MMU_addr regression: ${tag}"
    echo "=============================================================="
    rm -rf "$objdir"
    "$VERILATOR" --cc --exe --build -j 0 \
        $WARN $defs \
        --top-module MMU_addr \
        --Mdir "$objdir" \
        -CFLAGS "$defs" \
        $RTL_SRC tb_mmu_addr.cpp
    ./"$objdir"/VMMU_addr
}

rc=0
run_variant "default" "" || rc=$?
run_variant "full"    "-DFULL_MMU_ADDR" || rc=$?

if [ "$rc" -ne 0 ]; then
    echo "RESULT: FAILURES DETECTED"
    exit 1
fi
echo "RESULT: ALL MMU_addr REGRESSION TESTS PASSED"
