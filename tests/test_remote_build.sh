#!/usr/bin/env bash
# CPLD-BLD-0001: no network/toolchain required; fake ssh, rsync, yosys/fitter.
set -Eeuo pipefail
project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)
test_root=$(mktemp -d)
trap 'rm -rf "$test_root"' EXIT
export HOME="$test_root/home"
export PATH="$test_root/bin:$PATH"
export RTL_BUILD_HOST=fake-rtl
mkdir -p "$HOME/Tools/atf15xx_yosys/vendor" "$test_root/bin"
fixture="$test_root/project"
mkdir -p "$fixture/scripts" "$fixture/rtl_MMU" "$fixture/rtl_IOU"
cp "$project_root/scripts/"*.sh "$fixture/scripts/"
printf 'module MMU; endmodule\n' > "$fixture/rtl_MMU/MMU.v"
printf 'module MMU_part; endmodule\n' > "$fixture/rtl_MMU/MMU_part.v"
printf 'module IOU; endmodule\n' > "$fixture/rtl_IOU/IOU.v"
printf 'module IOU_part; endmodule\n' > "$fixture/rtl_IOU/IOU_part.v"
# MMU_io in the real project instantiates these reusable hardware helpers,
# which currently reside in rtl_IOU rather than rtl_MMU.
for shared in TTL_ls138_2 TTL_ls259 IOU_exp16 IOU_toggle_d; do
    printf 'module %s; endmodule\n' "$shared" > "$fixture/rtl_IOU/$shared.v"
done
printf 'STALE EDIF\n' > "$fixture/rtl_MMU/MMU.edif"
printf 'not a physical top\n' > "$fixture/rtl_IOU_MMU_TB.v"

atf_root="$HOME/Tools/atf15xx_yosys"
printf 'lib\n' > "$atf_root/cells.lib"
printf 'map\n' > "$atf_root/techmap.v"
printf 'fitter\n' > "$atf_root/vendor/fit1508.exe"

cat > "$test_root/bin/ssh" <<'SH'
#!/usr/bin/env bash
set -e
if [[ ${1:-} == -o ]]; then shift 2; fi
[[ ${1:-} == fake-rtl ]] || exit 77
shift
[[ ${MOCK_SSH_FAIL:-0} == 0 ]] || exit 78
bash -c "$*"
SH
cat > "$test_root/bin/rsync" <<'SH'
#!/usr/bin/env bash
set -e
src=${@: -2:1}
dst=${@: -1}
src=${src#fake-rtl:}
dst=${dst#fake-rtl:}
mkdir -p "$dst"
if [[ " $* " == *' --include='* ]]; then
    for x in "$src"/*.v; do [[ -f $x ]] && cp "$x" "$dst/"; done
elif [[ " $* " == *' -- '* ]]; then
    for x in "$@"; do
        [[ "$x" == "$dst" || "$x" == -- || "$x" == -* ]] && continue
        if [[ -f "$x" ]]; then cp -a "$x" "$dst/"; fi
    done
else
    cp -a "$src"/. "$dst"/
fi
SH
cat > "$test_root/bin/yosys" <<'SH'
#!/usr/bin/env bash
set -e
if [[ ${1:-} == -V ]]; then echo 'Yosys MOCK 0.63'; exit; fi
[[ ${MOCK_YOSYS_FAIL:-0} == 0 ]] || { echo 'mock synth failure' >&2; exit 42; }
[[ ${1:-} == -Q && ${2:-} == -T && ${3:-} == -s && -f ${4:-} ]] || exit 43
script=$4
[[ $(grep -c '^read_verilog' "$script") == 1 ]] || exit 44
if grep -q '^hierarchy -check -top MMU$' "$script"; then
    top=MMU
    grep -q 'input/MMU_part.v' "$script"
    ! grep -q 'input/IOU.v' "$script"
    ! grep -q 'input/IOU_spi_iface.v' "$script"
    for shared in TTL_ls138_2 TTL_ls259 IOU_exp16 IOU_toggle_d; do
        grep -q "input/$shared.v" "$script"
    done
else
    top=IOU
    grep -q 'input/IOU_part.v' "$script"
    ! grep -q 'MMU' "$script"
fi
grep -q '^write_edif -lsbidx' "$script"
printf 'edif for %s\n' "$top" > "$top.edif"
SH
cat > "$test_root/bin/wine" <<'SH'
#!/usr/bin/env bash
[[ ${1:-} == --version ]] && echo 'wine MOCK' || exit 1
SH
cat > "$atf_root/run_fitter.sh" <<'SH'
#!/usr/bin/env bash
set -e
[[ ${1:-} == -d && ${2:-} == ATF1508ASV ]] || exit 51
[[ ${3:-} == -p && ${4:-} == PLCC84 ]] || exit 52
[[ ${5:-} == -s && ${6:-} == 15 ]] || exit 53
[[ ${7:-} == MMU || ${7:-} == IOU ]] || exit 54
[[ ${MOCK_FITTER_FAIL:-0} == 0 ]] || { echo 'mock fitter failure' >&2; exit 55; }
printf 'fitted %s\n' "$7" > "$7.fit"
printf 'JED %s\n' "$7" > "$7.jed"
printf 'pin assignments\n' > "$7.pin"
SH
chmod +x "$test_root/bin/"* "$atf_root/run_fitter.sh"

pass=0
note() { echo "PASS $1"; pass=$((pass+1)); }
run() { (cd "$fixture" && ./scripts/build-cpld.sh "$@") > "$test_root/last.log" 2>&1; }
fail() { if run "$@"; then cat "$test_root/last.log"; echo "Expected failure: $*" >&2; exit 1; fi; }

run mmu
mapfile -t dirs < <(find "$fixture/build/rtl-cpld" -mindepth 1 -maxdepth 1 -type d -name 'MMU.*')
[[ ${#dirs[@]} == 1 && -s ${dirs[0]}/MMU.edif && -s ${dirs[0]}/MMU.jed ]]
[[ -s ${dirs[0]}/sources.sha256 && -s ${dirs[0]}/toolchain.txt && -s ${dirs[0]}/synth.ys ]]
[[ -f ${dirs[0]}/MMU.pin ]]
[[ $(cat "${dirs[0]}/MMU.edif") == 'edif for MMU' ]]
[[ ! -f ${dirs[0]}/IOU.jed ]]
note 'MMU: reads MMU sources plus four shared helpers, excludes IOU top, fits ASV and returns artifacts'

CPLD_IOU_DEFINES=IOU_COMPLETE run iou
mapfile -t ios < <(find "$fixture/build/rtl-cpld" -mindepth 1 -maxdepth 1 -type d -name 'IOU.*')
[[ ${#ios[@]} == 1 && -s ${ios[0]}/IOU.fit && -s ${ios[0]}/IOU.jed ]]
grep -q -- '-DIOU_COMPLETE' "${ios[0]}/synth.ys"
[[ ! -f ${ios[0]}/MMU.edif ]]
note 'IOU: uses independent source files and explicit IOU_COMPLETE define'

CPLD_MMU_DEFINES=FULL_MMU_ADDR run mmu
grep -ql -- '-DFULL_MMU_ADDR' "$fixture"/build/rtl-cpld/MMU.*/synth.ys
note 'MMU: optional FULL_MMU_ADDR included in synthesis'

CPLD_MMU_DEFINES=SPK_IN_MMU run mmu
grep -ql -- '-DSPK_IN_MMU' "$fixture"/build/rtl-cpld/MMU.*/synth.ys
note 'MMU: SPK_IN_MMU variant includes shared IOU_toggle_d helper'

# Missing dependencies must fail loudly rather than producing a silently
# incomplete netlist. Restore the file for subsequent all/failure tests.
mv "$fixture/rtl_IOU/TTL_ls259.v" "$fixture/rtl_IOU/TTL_ls259.v.hidden"
fail mmu
grep -q 'Missing shared MMU dependency.*TTL_ls259.v' "$test_root/last.log"
mv "$fixture/rtl_IOU/TTL_ls259.v.hidden" "$fixture/rtl_IOU/TTL_ls259.v"
note 'MMU: missing cross-directory shared module fails clearly'

run all
[[ $(find "$fixture/build/rtl-cpld" -mindepth 1 -maxdepth 1 -type d -name 'MMU.*' | wc -l) == 4 ]]
[[ $(find "$fixture/build/rtl-cpld" -mindepth 1 -maxdepth 1 -type d -name 'IOU.*' | wc -l) == 2 ]]
note 'all: two independent builds; non-overwriting per-run output directories'

fail unrelated
fail mmu extra
CPLD_MMU_DEFINES='BAD;rm' fail mmu
RTL_BUILD_HOST='bad;host' fail mmu
note 'invalid targets, options, defines and hosts rejected'

MOCK_SSH_FAIL=1 fail mmu
note 'SSH failure returns nonzero'

MOCK_YOSYS_FAIL=1 fail mmu
latest=$(find "$fixture/build/rtl-cpld" -mindepth 1 -maxdepth 1 -type d -name 'MMU.*' -printf '%T@ %p\n' | sort -nr | head -1 | cut -d' ' -f2-)
[[ -f "$latest/yosys.log" && ! -e "$latest/MMU.jed" ]]
note 'Yosys failure retains diagnostics without presenting a stale JEDEC'

MOCK_FITTER_FAIL=1 fail iou
latest=$(find "$fixture/build/rtl-cpld" -mindepth 1 -maxdepth 1 -type d -name 'IOU.*' -printf '%T@ %p\n' | sort -nr | head -1 | cut -d' ' -f2-)
[[ -f "$latest/fitter.log" && -s "$latest/IOU.edif" && ! -e "$latest/IOU.jed" ]]
note 'fitter failure retains EDIF and diagnostics, returns nonzero'

echo "All $pass remote-build mock checks passed. No remote host or real fitter was used."
