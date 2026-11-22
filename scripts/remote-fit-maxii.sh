#!/usr/bin/env bash
# CPLD-BLD-0002: streamed over SSH and run INSIDE devel-rtl.
set -Eeuo pipefail

[[ $# == 3 ]] || { echo "Expected WORKDIR PART VARIANT" >&2; exit 2; }
workdir=$1
part=$2
variant=$3
case "$part" in
    EPM240T100C5|EPM570T100C5|EPM570T144C5) ;;
    *) echo "Unsupported MAX II target: $part" >&2; exit 2 ;;
esac
[[ "$variant" == compact || "$variant" == full ]] || {
    echo "Invalid address variant" >&2; exit 2;
}
[[ -d "$workdir/input/rtl_single" && -d "$workdir/input/quartus/maxii" ]] || {
    echo "Incomplete source transfer" >&2; exit 2;
}
cd "$workdir"
mkdir -p artifacts
project_dir="$workdir/input/quartus/maxii"

# Always retain diagnostics and the effective project settings, on success OR
# failure. Generated files are copied OUT of the Quartus working directory;
# they never enter devel's Git checkout or contaminate a later build.
collect_artifacts() {
    local file
    shopt -s nullglob
    for file in "$project_dir"/cpld_top.{qpf,qsf,sdc} \
                "$project_dir"/*.rpt "$project_dir"/*.summary \
                "$project_dir"/output_files/*.{rpt,summary,pof,pin,jdi,smsg}; do
        [[ -f "$file" ]] || continue
        cp -f -- "$file" "$workdir/artifacts/" || true
    done
    shopt -u nullglob
}
trap collect_artifacts EXIT

[[ -s "$project_dir/cpld_top.qpf" && -s "$project_dir/cpld_top.qsf" && \
   -s "$project_dir/cpld_top.sdc" && -s "$project_dir/configure.tcl" ]] || {
    echo "Missing tracked Quartus project file" >&2; exit 2;
}
shopt -s nullglob
rtl_sources=(input/rtl_single/*.v)
shopt -u nullglob
(("${#rtl_sources[@]}" >= 17)) || {
    echo "Incomplete rtl_single source set (expected at least 17 Verilog files)" >&2
    exit 2
}
[[ -s input/rtl_single/cpld_top.v ]] || { echo "Missing cpld_top" >&2; exit 2; }

# Strongly prefer the installation verified on Fedora devel-rtl. A non-
# interactive SSH shell does NOT necessarily source ~/.bashrc.
if ! command -v quartus_sh >/dev/null 2>&1; then
    for candidate in "$HOME/Tools/intelFPGA_lite/25.1std/quartus/bin" \
                     "$HOME/intelFPGA_lite/25.1std/quartus/bin"; do
        if [[ -x "$candidate/quartus_sh" ]]; then
            export PATH="$candidate:$PATH"
            export QUARTUS_ROOTDIR="$candidate/.."
            break
        fi
    done
fi
command -v quartus_sh >/dev/null 2>&1 || {
    echo "Quartus not found in PATH or documented 25.1 installation" >&2
    exit 2
}

{
    echo "Target: $part"
    echo "Top: cpld_top"
    echo "Variant: $variant"
    echo "Host: $(hostname)"
    echo "Workdir: $workdir"
    quartus_sh --version
    quartus_map --version
    quartus_fit --version
    quartus_sta --version
} > artifacts/toolchain.txt 2>&1
sha256sum "${rtl_sources[@]}" input/quartus/maxii/*.{qpf,qsf,sdc,tcl} \
    > artifacts/sources.sha256

cd "$project_dir"
if ! quartus_sh -t configure.tcl "$part" "$variant" \
    > "$workdir/artifacts/configure.log" 2>&1; then
    echo "Quartus project configuration failed (configure.log)" >&2
    exit 1
fi

# Standard full-flow compilation includes map, fit, assembler (POF) and STA
# as supported by this device. No simulator or device-programmer is needed.
if ! quartus_sh --flow compile cpld_top \
    > "$workdir/artifacts/compile.log" 2>&1; then
    echo "Quartus compilation failed (compile.log and available *.rpt)" >&2
    exit 1
fi
if [[ ! -s output_files/cpld_top.pof ]]; then
    echo "Quartus completed without expected nonempty MAX II POF" >&2
    exit 1
fi
echo "Quartus built $part ($variant); timing and pinout NOT approved."
