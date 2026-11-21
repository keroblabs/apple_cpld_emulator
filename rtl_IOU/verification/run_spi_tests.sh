#!/usr/bin/env bash
set -euo pipefail

tests_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "$tests_dir/../.." && pwd)"
build_dir="$(mktemp -d)"
trap 'rm -rf -- "$build_dir"' EXIT

export VERILATOR_ROOT="${VERILATOR_ROOT:-$HOME/Tools/verilator}"
"$VERILATOR_ROOT/bin/verilator" --cc --exe --build -j 2 -Wall --trace \
    --top-module IOU_spi_iface --Mdir "$build_dir/obj" \
    -I"$repo_dir/rtl_IOU" \
    "$repo_dir/rtl_IOU/IOU_spi_iface.v" \
    "$tests_dir/test_spi_iface.cpp"

waveform="${SPI_WAVEFORM:-$build_dir/spi_iface_waveform.vcd}"
"$build_dir/obj/VIOU_spi_iface" "$waveform"
