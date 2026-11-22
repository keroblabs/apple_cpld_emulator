# Single-CPLD MAX II remote Quartus build — CPLD-BLD-0002

**Status:** initial build infrastructure in REVIEW; **no real fitter run is claimed**.
**Toolchain:** Quartus Prime Lite 25.1std.0 build 1129, MAX II support, installed on `devel-rtl` at `~/Tools/intelFPGA_lite/25.1std`. The official source checkout, editor, Git and Verilator remain on `devel`.

This is **separate** from the working ATF1508ASV scripts described by [remote-rtl-build.md](remote-rtl-build.md). Do not replace the older `scripts/build-cpld.sh` path.

## Targets and variants

| Build command | Device | RTL configuration | Purpose |
| --- | --- | --- | --- |
| `bash scripts/build-maxii.sh epm240` | `EPM240T100C5` | Compact, 5-bit `MEM_ADDR_OUT` | First feasibility fit |
| `bash scripts/build-maxii.sh epm240 full` | `EPM240T100C5` | Full 17-bit memory address | I/O-constrained experiment |
| `bash scripts/build-maxii.sh epm570` | `EPM570T100C5` | Compact | Higher-logic fallback |
| `bash scripts/build-maxii.sh epm570t144 full` | `EPM570T144C5` | Full | Higher-pin-count fallback |

The existing single-chip `rtl_single/cpld_top.v` deliberately omits SW2, PDL2 and PDL3 and ties their C06x inputs low. It exposes **64 logical I/O signals** in compact mode and **76** in full mode; neither count proves a particular package can fit. Check the real development board's part marking and pin availability before assigning pads.

## Tracked project inputs

`quartus/maxii/cpld_top.qpf`: project revision.
`quartus/maxii/cpld_top.qsf`: top entity, MAX II family, explicit source list, output directory and experimental SDC, **without any invented PCB pin assignments**.
`quartus/maxii/cpld_top.sdc`: illustrative 1 MHz PHI_0 and 125 kHz SPI SCK clocks matching prior digital phase-sweep assumptions; this is **not** verified physical clock/timing sign-off.
`quartus/maxii/configure.tcl`: sets exact target part and optional `FULL_MMU_ADDR` macro inside an **isolated copy** of the project.

All RTL is taken from `rtl_single/*.v`. No old MMU/IOU tops, `obj_dir`, ignored EDIF, JED or historic fitter artifacts are inputs.

## Prerequisites

From `devel`:

```bash
ssh -o BatchMode=yes devel-rtl 'hostname; ~/Tools/intelFPGA_lite/25.1std/quartus/bin/quartus_sh --version'
command -v rsync
bash -n scripts/build-maxii.sh scripts/remote-fit-maxii.sh tests/test_remote_maxii_build.sh
bash tests/test_remote_maxii_build.sh
```

`quartus_sh`, `quartus_map`, `quartus_fit` and `quartus_sta` must work on the build VM. `libxcrypt-compat` was necessary for `quartus_map` on Fedora 42. The remote script adds `~/Tools/intelFPGA_lite/25.1std/quartus/bin` to PATH when SSH's noninteractive shell does not load `~/.bashrc`.

## First real build

```bash
# Execute on devel, at the apple_cpld_emulator repo checkout.
bash scripts/build-maxii.sh epm240

# Optional second test, only after inspecting the first run:
bash scripts/build-maxii.sh epm570
```

`RTL_BUILD_HOST` may override the SSH host alias (hostname characters only). Each invocation creates an isolated `~/rtl-build/src/maxii.cpld_top.*` remote workspace and streams `scripts/remote-fit-maxii.sh` over SSH. It configures the tracked project, runs `quartus_sh --flow compile cpld_top`, and copies back results even on failure.

Output is unique and ignored by Git, for example:

```text
build/rtl-maxii/EPM240T100C5_compact_20261122_231500/
    cpld_top.qpf
    cpld_top.qsf         # effective part and defines from Quartus
    cpld_top.sdc
    configure.log
    compile.log
    toolchain.txt
    sources.sha256
    cpld_top.map.rpt     # when generated
    cpld_top.fit.rpt     # when generated
    cpld_top.sta.rpt     # when generated
    cpld_top.pin         # when generated
    cpld_top.pof         # only on successful full compilation
```

The local timestamp is obtained from `devel` using its own clock in `YYYYMMDD_HHMMSS` format. Concurrent/same-second builds get `_01`, `_02`, etc. Remote intermediate databases are retained for investigation. A **failed** compile returns nonzero and saves available logs/reports; a historical or absent `.pof` cannot count as a pass.

## Acceptance and interpretation

1. Run the no-network mock tests above, record their exact output. They check script transfer, target selection, compact/full macro, collision isolation, failure propagation and diagnostics, **not functional RTL or real Quartus**.
2. Perform the real EPM240 compact build on `devel-rtl` and inspect `compile.log`, `cpld_top.map.rpt`, `cpld_top.fit.rpt`, output pin file and `cpld_top.sta.rpt`. Record exact LE/register/pin counts, warnings, fitted or failed result, and the `sources.sha256` set in [CPLD-BLD-0002](tasks/db/CPLD-BLD-0002.md) and [CPLD-RTL-0001](tasks/db/CPLD-RTL-0001.md).
3. Do not assert timing closure from the sample clocks. Real board-specific setup/hold, reset, SPI-to-PHI_0 CDC, I/O timing, voltage, clock routing, bus isolation and pin assignment still belong to [CPLD-HW-0006](tasks/db/CPLD-HW-0006.md).
4. Treat the `.pof` as a **generated artifact only**, not authorization to program an unknown AliExpress board. Verify the part marking, JTAG connection, power rail and pinout before hardware use.

After reviewing EPM240 utilization, decide whether any RTL optimization is justified or the EPM570 is the better physical device. Preserve Verilator functional regressions for every RTL change.
