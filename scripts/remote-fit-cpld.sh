#!/usr/bin/env bash
# CPLD-BLD-0001: runs via ssh bash -s on the RTL build VM, not on devel.
set -Eeuo pipefail

[[ $# == 3 ]] || { echo 'Expected TOP WORKDIR DEFINES' >&2; exit 2; }
top=$1
workdir=$2
defines=$3
[[ "$top" == MMU || "$top" == IOU ]] || { echo 'Unknown physical CPLD top' >&2; exit 2; }
[[ -d "$workdir/input" ]] || { echo 'Missing transferred input directory' >&2; exit 2; }

atf_root=${ATF15XX_ROOT:-"$HOME/Tools/atf15xx_yosys"}
[[ -s "$atf_root/cells.lib" && -s "$atf_root/techmap.v" && -x "$atf_root/run_fitter.sh" ]] || {
    echo "Incomplete atf15xx_yosys installation: $atf_root" >&2
    exit 2
}
command -v yosys >/dev/null || { echo 'yosys is missing' >&2; exit 2; }

cd "$workdir"
mkdir -p artifacts
# Always preserve intermediate outputs and logs, including on fitter failure.
collect_artifacts() {
    local extension
    for extension in edif fit jed pin io tt3; do
        if [[ -f "$top.$extension" ]]; then
            cp -f -- "$top.$extension" artifacts/
        fi
    done
}
trap collect_artifacts EXIT

shopt -s nullglob
sources=(input/*.v)
shopt -u nullglob
((${#sources[@]} > 0)) || { echo 'No RTL source files transferred' >&2; exit 2; }
[[ -f "input/$top.v" ]] || { echo "Missing physical top: $top.v" >&2; exit 2; }

# The Atmel flow needs every submodule, unlike upstream run_yosys.sh which
# reads only TOP.v. Paths and filenames in the actual project contain no spaces.
for source in "${sources[@]}"; do
    [[ "$source" =~ ^input/[A-Za-z0-9_]+\.v$ ]] || {
        echo "Unsafe Verilog source filename: $source" >&2
        exit 2
    }
done

{
    printf 'Target: ATF1508ASV-15JU84 (PLCC84, 15 ns)\n'
    printf 'Top: %s\nDefines: %s\n' "$top" "${defines:-<none>}"
    yosys -V
    wine --version
    sha256sum "$atf_root/vendor/fit1508.exe" 2>/dev/null || true
} > artifacts/toolchain.txt 2>&1
sha256sum "${sources[@]}" > artifacts/sources.sha256

{
    printf 'read_liberty -lib %s/cells.lib\n' "$atf_root"
    printf 'read_verilog -sv'
    for define in $defines; do
        [[ "$define" =~ ^[A-Za-z_][A-Za-z_0-9]*$ ]] || { echo "Invalid define: $define" >&2; exit 2; }
        printf ' -D%s' "$define"
    done
    printf ' %s' "${sources[@]}"
    printf '\n'
    printf 'hierarchy -check -top %s\n' "$top"
    printf 'tribuf\n'
    printf 'synth -flatten -noabc -top %s\n' "$top"
    printf 'techmap -map %s/techmap.v -D skip_DFFE_XX_\n' "$atf_root"
    printf 'simplemap\n'
    printf 'dfflibmap -liberty %s/cells.lib\n' "$atf_root"
    printf 'abc -liberty %s/cells.lib\n' "$atf_root"
    printf 'iopadmap -bits -inpad INBUF Q:A -outpad BUF A:Q -toutpad TRI ENA:A:Q -tinoutpad bibuf EN:Q:A:PAD\n'
    printf 'clean\n'
    printf 'splitnets -format _\n'
    printf 'rename -wire -suffix _reg t:*DFF*\n'
    printf 'rename -wire -suffix _comb\n'
    printf 'delete t:$scopeinfo\n'
    printf 'write_edif -lsbidx %s.edif\n' "$top"
} > synth.ys
cp synth.ys artifacts/

if ! yosys -Q -T -s synth.ys > artifacts/yosys.log 2>&1; then
    echo "Yosys failed for $top: artifacts/yosys.log" >&2
    exit 1
fi
[[ -s "$top.edif" ]] || { echo 'Yosys produced no EDIF' >&2; exit 1; }

if ! "$atf_root/run_fitter.sh" -d ATF1508ASV -p PLCC84 -s 15 "$top" \
    > artifacts/fitter.log 2>&1; then
    echo "Atmel fitter failed for $top: artifacts/fitter.log" >&2
    exit 1
fi
[[ -s "$top.fit" && -s "$top.jed" ]] || {
    echo 'Fitter did not produce nonempty FIT and JEDEC outputs' >&2
    exit 1
}
