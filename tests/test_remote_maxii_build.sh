#!/usr/bin/env bash
# CPLD-BLD-0002: isolated mock SSH/rsync/Quartus test. NEVER invokes a real VM.
set -Eeuo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
export HOME="$tmp/home" PATH="$tmp/bin:$PATH" RTL_BUILD_HOST=fake-rtl
mkdir -p "$HOME" "$tmp/bin" "$tmp/repo/scripts" "$tmp/repo/rtl_single" "$tmp/repo/quartus/maxii"
cp "$root"/rtl_single/*.v "$tmp/repo/rtl_single/"
cp "$root"/quartus/maxii/*.{qpf,qsf,sdc,tcl} "$tmp/repo/quartus/maxii/"
cp "$root/scripts/build-maxii.sh" "$root/scripts/remote-fit-maxii.sh" "$tmp/repo/scripts/"
touch "$tmp/repo/rtl_single/old.pof" "$tmp/repo/rtl_single/old.edif"

cat > "$tmp/bin/date" <<'SH'
#!/usr/bin/env bash
[[ ${1:-} == '+%Y%m%d_%H%M%S' ]] || exit 65
printf '%s\n' "${MOCK_TIMESTAMP:-20261122_231500}"
SH
cat > "$tmp/bin/ssh" <<'SH'
#!/usr/bin/env bash
set -e
[[ ${1:-} == -o ]] && shift 2
[[ ${1:-} == fake-rtl ]] || exit 77
shift
[[ ${MOCK_SSH_FAIL:-0} == 0 ]] || exit 78
bash -c "$*"
SH
cat > "$tmp/bin/rsync" <<'SH'
#!/usr/bin/env bash
set -e
src=${@: -2:1}
dst=${@: -1}
src=${src#fake-rtl:}
dst=${dst#fake-rtl:}
mkdir -p "$dst"
if [[ $src == */rtl_single/ ]]; then
    for f in "$src"/*.v; do cp "$f" "$dst/"; done
elif [[ $src == */quartus/maxii/ ]]; then
    for f in "$src"/*.{qpf,qsf,sdc,tcl}; do cp "$f" "$dst/"; done
else
    cp -a "$src"/. "$dst/"
fi
SH
cat > "$tmp/bin/quartus_sh" <<'SH'
#!/usr/bin/env bash
set -e
case "${1:-}" in
    --version) echo 'Quartus MOCK 25.1std'; exit 0 ;;
    -t)
        [[ ${2:-} == configure.tcl ]] || exit 50
        [[ ${MOCK_CONFIG_FAIL:-0} == 0 ]] || exit 51
        printf 'set_global_assignment -name DEVICE %s\n' "$3" >> cpld_top.qsf
        if [[ $4 == full ]]; then
            printf 'set_global_assignment -name VERILOG_MACRO FULL_MMU_ADDR=1\n' >> cpld_top.qsf
        fi
        exit 0 ;;
    --flow)
        [[ ${2:-} == compile && ${3:-} == cpld_top ]] || exit 52
        [[ -f ../../rtl_single/cpld_top.v ]] || exit 53
        mkdir -p output_files
        if [[ ${MOCK_COMPILER_FAIL:-0} != 0 ]]; then
            echo 'mock fitter failed' > output_files/cpld_top.fit.rpt
            exit 54
        fi
        echo 'mock fitter passed' > output_files/cpld_top.fit.rpt
        echo 'mock STA (not signoff)' > output_files/cpld_top.sta.rpt
        if [[ ${MOCK_MISSING_POF:-0} == 0 ]]; then
            echo 'mock MAX II POF' > output_files/cpld_top.pof
        fi
        exit 0 ;;
esac
exit 55
SH
for exe in quartus_map quartus_fit quartus_sta; do
    cat > "$tmp/bin/$exe" <<'SH'
#!/usr/bin/env bash
[[ ${1:-} == --version ]] || exit 60
echo "Quartus MOCK"
SH
done
chmod +x "$tmp/bin/"*

run() { (cd "$tmp/repo" && bash scripts/build-maxii.sh "$@") >"$tmp/last.log" 2>&1; }
expect_fail() {
    if run "$@"; then echo "Expected failure: $*" >&2; cat "$tmp/last.log"; exit 1; fi
}
check() { echo "PASS: $1"; }

run epm240
out="$tmp/repo/build/rtl-maxii/EPM240T100C5_compact_20261122_231500"
[[ -s "$out/cpld_top.pof" && -s "$out/compile.log" && -s "$out/configure.log" ]]
grep -q 'DEVICE EPM240T100C5' "$out/cpld_top.qsf"
[[ ! -e "$out/old.pof" && ! -e "$out/old.edif" ]]
[[ -s "$out/sources.sha256" && -s "$out/toolchain.txt" ]]
check 'EPM240 compact, correct QSF and filtered sources, artifacts returned'

run epm240 full
out_full="$tmp/repo/build/rtl-maxii/EPM240T100C5_full_20261122_231500"
grep -q 'FULL_MMU_ADDR=1' "$out_full/cpld_top.qsf"
[[ -s "$out_full/cpld_top.pof" ]]
check 'EPM240 full address define applied in isolated project'

run epm570t144 full
out_570="$tmp/repo/build/rtl-maxii/EPM570T144C5_full_20261122_231500"
grep -q 'DEVICE EPM570T144C5' "$out_570/cpld_top.qsf"
check 'EPM570T144 fallback part selected'

run epm240
[[ -s "$tmp/repo/build/rtl-maxii/EPM240T100C5_compact_20261122_231500_01/cpld_top.pof" ]]
check 'timestamp collisions never overwrite existing output'

expect_fail invalid
expect_fail epm240 unexpected
RTL_BUILD_HOST='host;bad' expect_fail epm240
check 'invalid targets/variants/host rejected'

MOCK_COMPILER_FAIL=1 expect_fail epm570
failed="$tmp/repo/build/rtl-maxii/EPM570T100C5_compact_20261122_231500"
[[ -s "$failed/compile.log" && -s "$failed/cpld_top.fit.rpt" && ! -e "$failed/cpld_top.pof" ]]
check 'failed fitter returns nonzero with diagnostic report, no stale POF'

MOCK_CONFIG_FAIL=1 expect_fail epm570
[[ -s "$tmp/repo/build/rtl-maxii/EPM570T100C5_compact_20261122_231500_01/configure.log" ]]
check 'configuration failure retains log'

MOCK_MISSING_POF=1 expect_fail epm570
check 'no false successful fit when POF missing'

MOCK_SSH_FAIL=1 expect_fail epm240
check 'SSH failure returns nonzero'

mv "$tmp/repo/rtl_single/slot_control.v" "$tmp/repo/rtl_single/slot_control.v.hidden"
expect_fail epm240
grep -q 'Missing Quartus RTL dependency' "$tmp/last.log"
check 'missing RTL helper rejected before transfer'

echo "All mock MAX II build checks passed. No real Quartus compile was performed."
