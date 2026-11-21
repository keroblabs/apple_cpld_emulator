#!/usr/bin/env bash
set -euo pipefail

tests_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "$tests_dir/../.." && pwd)"
build_dir="$(mktemp -d)"
trap 'rm -rf -- "$build_dir"' EXIT

export VERILATOR_ROOT="${VERILATOR_ROOT:-$HOME/Tools/verilator}"
"$VERILATOR_ROOT/bin/verilator" --cc --exe --build -j 2 -Wall --trace \
    --timescale-override 1ns/1ns \
    -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM -Wno-PINCONNECTEMPTY \
    --top-module IOU_spi_phase_tb --Mdir "$build_dir/obj" \
    -I"$repo_dir/rtl_IOU" \
    "$repo_dir/rtl_IOU/IOU_toggle_jk.v" \
    "$repo_dir/rtl_IOU/IOU_iomem.v" \
    "$repo_dir/rtl_IOU/IOU_spi_iface.v" \
    "$tests_dir/IOU_spi_phase_tb.v" \
    "$tests_dir/test_spi_phase.cpp"

waveform="${SPI_PHASE_WAVEFORM:-$build_dir/spi_phase_handshake.vcd}"
"$build_dir/obj/VIOU_spi_phase_tb" "$waveform"
