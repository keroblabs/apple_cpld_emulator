#!/usr/bin/env bash
# CPLD-BLD-0001: run an isolated MMU/IOU synthesis and fit on devel-rtl.
set -Eeuo pipefail

usage() {
    cat >&2 <<'HELP'
Usage: scripts/build-cpld.sh {mmu|iou|all}

Build each physical CPLD independently on devel-rtl, returning results to
build/rtl-cpld/<TOP>_YYYYMMDD_HHMMSS/ (ignored by Git).
If a build starts in the same second, use suffix _01, _02, and so on.

Environment:
  RTL_BUILD_HOST       SSH host alias (default: devel-rtl)
  CPLD_MMU_DEFINES     Space-separated Verilog defines for the MMU
  CPLD_IOU_DEFINES     Space-separated Verilog defines for the IOU

Examples:
  ./scripts/build-cpld.sh mmu
  CPLD_MMU_DEFINES=FULL_MMU_ADDR ./scripts/build-cpld.sh mmu
  CPLD_IOU_DEFINES=IOU_COMPLETE ./scripts/build-cpld.sh iou
HELP
    exit 2
}

[[ $# == 1 ]] || usage
case "$1" in
    mmu) targets=(MMU) ;;
    iou) targets=(IOU) ;;
    all) targets=(MMU IOU) ;;
    *) usage ;;
esac

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)
host=${RTL_BUILD_HOST:-devel-rtl}
[[ "$host" =~ ^[a-zA-Z0-9_.-]+$ ]] || { echo 'Invalid RTL_BUILD_HOST' >&2; exit 2; }

# Explicit source configurations; the physical HW-0004 variant is not assumed.
validate_defines() {
    local words=$1 token
    for token in $words; do
        [[ "$token" =~ ^[a-zA-Z_][a-zA-Z_0-9]*$ ]] || {
            echo "Invalid Verilog define: $token" >&2
            return 2
        }
    done
}
validate_defines "${CPLD_MMU_DEFINES:-}"
validate_defines "${CPLD_IOU_DEFINES:-}"

mkdir -p "$root/build/rtl-cpld"

# Local timestamps are taken on devel. mkdir is atomic, so a concurrent
# invocation cannot overwrite another run using the same timestamp.
new_result_dir() {
    local top=$1 stamp base candidate suffix=0
    stamp=$(date '+%Y%m%d_%H%M%S') || return
    [[ "$stamp" =~ ^[0-9]{8}_[0-9]{6}$ ]] || {
        echo "Invalid build timestamp: $stamp" >&2
        return 1
    }
    base="$root/build/rtl-cpld/${top}_${stamp}"
    candidate=$base
    while ! mkdir -- "$candidate" 2>/dev/null; do
        if [[ ! -d "$candidate" ]]; then
            echo "Cannot create build output directory: $candidate" >&2
            return 1
        fi
        suffix=$((suffix + 1))
        if (( suffix > 999 )); then
            echo "Too many builds for timestamp $stamp" >&2
            return 1
        fi
        printf -v candidate '%s_%02d' "$base" "$suffix"
    done
    printf '%s\n' "$candidate"
}

build_one() {
    local top=$1 source_dir="$root/rtl_$1" defines remote_dir result_dir run_cmd rc=0
    local shared_name shared_path
    local -a shared_sources=()
    [[ -f "$source_dir/$top.v" ]] || { echo "Missing $source_dir/$top.v" >&2; return 2; }
    if [[ "$top" == MMU ]]; then
        defines=${CPLD_MMU_DEFINES:-}
        # MMU_io instantiates these shared decoders/latches, which currently
        # live in rtl_IOU. Include only these modules, not the IOU top or SPI.
        # IOU_toggle_d is needed when SPK_IN_MMU is defined.
        for shared_name in TTL_ls138_2 TTL_ls259 IOU_exp16 IOU_toggle_d; do
            shared_path="$root/rtl_IOU/$shared_name.v"
            [[ -f "$shared_path" ]] || {
                echo "Missing shared MMU dependency: $shared_path" >&2
                return 2
            }
            shared_sources+=("$shared_path")
        done
    else
        defines=${CPLD_IOU_DEFINES:-}
    fi

    # One remote directory per attempt; never reuse an existing .edif/.jed.
    remote_dir=$(ssh -o BatchMode=yes "$host" \
        "mkdir -p \"\$HOME/rtl-build/src\" && mktemp -d \"\$HOME/rtl-build/src/cpld.${top}.XXXXXXXX\"") || return
    # Only trust mktemp-shaped paths. Never construct a cleanup command from
    # unvalidated remote output.
    [[ "$remote_dir" =~ ^/[a-zA-Z0-9_./-]+/rtl-build/src/cpld\.${top}\.[a-zA-Z0-9]+$ ]] || {
        echo "Unexpected remote build path: $remote_dir" >&2
        return 2
    }
    ssh -o BatchMode=yes "$host" "mkdir -p '$remote_dir/input'"

    # The chosen CPLD top is the only physical design; the MMU additionally
    # imports four specific shared helper modules from rtl_IOU.
    # Historical EDIF/JED, verification and the Verilator top are excluded.
    rsync -a --include='*.v' --exclude='*' \
        "$source_dir/" "$host:$remote_dir/input/"
    if ((${#shared_sources[@]} > 0)); then
        rsync -a -- "${shared_sources[@]}" "$host:$remote_dir/input/"
    fi
    result_dir=$(new_result_dir "$top") || return

    # shell-escaped arguments for ssh's remote shell. The build script itself
    # is streamed via stdin, so it is always the version in this checkout.
    printf -v run_cmd 'bash -s -- %q %q %q' "$top" "$remote_dir" "$defines"
    if ssh -o BatchMode=yes "$host" "$run_cmd" < "$root/scripts/remote-fit-cpld.sh"; then
        rc=0
    else
        rc=$?
        echo "Remote $top build failed (exit $rc); retrieving diagnostics" >&2
    fi

    # Retrieve diagnostics on both success and failure; never mask a build error.
    if ! rsync -a "$host:$remote_dir/artifacts/" "$result_dir/"; then
        echo "Failed to retrieve $top results from $remote_dir" >&2
        return 1
    fi
    if (( rc != 0 )); then
        echo "Failed build logs: $result_dir" >&2
        return "$rc"
    fi
    for suffix in edif fit jed; do
        if [[ ! -s "$result_dir/$top.$suffix" ]]; then
            echo "Incomplete $top build: missing or empty $top.$suffix in $result_dir" >&2
            return 1
        fi
    done
    echo "$top generated artifacts: $result_dir"
    echo "Remote work directory retained for inspection: $remote_dir"
}

for target in "${targets[@]}"; do
    build_one "$target"
done
