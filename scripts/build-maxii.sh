#!/usr/bin/env bash
# CPLD-BLD-0002: compile the single-chip cpld_top with Quartus on devel-rtl.
# Run from the source-of-truth checkout in devel; NEVER build in that source tree.
set -Eeuo pipefail

usage() {
    cat >&2 <<'HELP'
Usage: ./scripts/build-maxii.sh {epm240|epm570|epm570t144} [compact|full]

Build the tracked rtl_single/cpld_top.v for a MAX II device on devel-rtl.
The default is compact (5-bit MEM_ADDR_OUT); full enables FULL_MMU_ADDR.
No board pin assignments are made. These builds establish logic feasibility,
not PCB timing closure or programming approval.

Environment:
  RTL_BUILD_HOST   SSH host alias (default devel-rtl)

Results are kept in ignored build/rtl-maxii/PART_VARIANT_YYYYMMDD_HHMMSS/
with collision-safe _01, _02 ... suffixes.

Examples:
  ./scripts/build-maxii.sh epm240
  ./scripts/build-maxii.sh epm240 full
  ./scripts/build-maxii.sh epm570
  ./scripts/build-maxii.sh epm570t144 full
HELP
    exit 2
}

[[ $# -ge 1 && $# -le 2 ]] || usage
case "$1" in
    epm240)     part=EPM240T100C5 ;;
    epm570)     part=EPM570T100C5 ;;
    epm570t144) part=EPM570T144C5 ;;
    *) usage ;;
esac
variant=${2:-compact}
[[ "$variant" == compact || "$variant" == full ]] || usage

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)
host=${RTL_BUILD_HOST:-devel-rtl}
[[ "$host" =~ ^[a-zA-Z0-9_.-]+$ ]] || {
    echo "Invalid RTL_BUILD_HOST: $host" >&2; exit 2;
}
[[ -f "$root/rtl_single/cpld_top.v" ]] || {
    echo 'Missing rtl_single/cpld_top.v' >&2; exit 2;
}
for required in cpld_top.qpf cpld_top.qsf cpld_top.sdc configure.tcl; do
    [[ -s "$root/quartus/maxii/$required" ]] || {
        echo "Missing quartus/maxii/$required" >&2; exit 2;
    }
done

# Check that ALL tracked Verilog sources listed in the project actually exist.
# Do not silently build with one missing helper or with ignored obj_dir content.
while IFS= read -r source; do
    [[ -f "$root/rtl_single/$source" ]] || {
        echo "Missing Quartus RTL dependency: rtl_single/$source" >&2; exit 2;
    }
done < <(sed -n 's|^set_global_assignment -name VERILOG_FILE ../../rtl_single/\([^[:space:]]*\)$|\1|p' "$root/quartus/maxii/cpld_top.qsf")

mkdir -p "$root/build/rtl-maxii"
new_result_dir() {
    local prefix=$1 stamp base candidate suffix=0
    stamp=$(date '+%Y%m%d_%H%M%S') || return
    [[ "$stamp" =~ ^[0-9]{8}_[0-9]{6}$ ]] || {
        echo "Invalid timestamp: $stamp" >&2; return 1;
    }
    base="$root/build/rtl-maxii/${prefix}_${stamp}"
    candidate=$base
    while ! mkdir -- "$candidate" 2>/dev/null; do
        [[ -d "$candidate" ]] || { echo "Cannot create $candidate" >&2; return 1; }
        suffix=$((suffix + 1))
        ((suffix <= 999)) || { echo "Too many builds in same second" >&2; return 1; }
        printf -v candidate '%s_%02d' "$base" "$suffix"
    done
    printf '%s\n' "$candidate"
}

# Each run has a unique REMOTE working directory. No stale Quartus db is reused.
remote_dir=$(ssh -o BatchMode=yes "$host" \
    'mkdir -p "$HOME/rtl-build/src" && mktemp -d "$HOME/rtl-build/src/maxii.cpld_top.XXXXXXXX"')
[[ "$remote_dir" =~ ^/[a-zA-Z0-9_./-]+/rtl-build/src/maxii\.cpld_top\.[a-zA-Z0-9]+$ ]] || {
    echo "Unexpected remote workspace: $remote_dir" >&2; exit 2;
}
ssh -o BatchMode=yes "$host" \
    "mkdir -p '$remote_dir/input/rtl_single' '$remote_dir/input/quartus/maxii'"

# Restrict transfer to RTL and tracked Quartus project files; NO object files,
# source-tree db, output_files, historical reports or programming artifacts.
rsync -a --include='*.v' --exclude='*' \
    "$root/rtl_single/" "$host:$remote_dir/input/rtl_single/"
rsync -a --include='*.qpf' --include='*.qsf' --include='*.sdc' --include='*.tcl' --exclude='*' \
    "$root/quartus/maxii/" "$host:$remote_dir/input/quartus/maxii/"

result_dir=$(new_result_dir "${part}_${variant}")
printf -v run_cmd 'bash -s -- %q %q %q' "$remote_dir" "$part" "$variant"
rc=0
if ssh -o BatchMode=yes "$host" "$run_cmd" < "$root/scripts/remote-fit-maxii.sh"; then
    rc=0
else
    rc=$?
    echo "MAX II build failed (exit $rc); retrieving diagnostics" >&2
fi
if ! rsync -a "$host:$remote_dir/artifacts/" "$result_dir/"; then
    echo "Could not retrieve diagnostics from $remote_dir" >&2
    exit 1
fi
if ((rc != 0)); then
    echo "Failed build artifacts: $result_dir" >&2
    exit "$rc"
fi

for required in compile.log configure.log sources.sha256 cpld_top.qsf cpld_top.pof; do
    [[ -s "$result_dir/$required" ]] || {
        echo "Incomplete MAX II build: missing $required in $result_dir" >&2
        exit 1
    }
done
echo "MAX II ${part} (${variant}) reports and POF: $result_dir"
echo "Remote workspace retained: $remote_dir"
echo "NOTE: Without final board pins/I-O delays/CDC constraints, timing is NOT signed off."
