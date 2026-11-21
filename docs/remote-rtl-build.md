# Remote ATF1508ASV builds — Apple CPLD emulator

**Ticket:** [CPLD-BLD-0001](tasks/db/CPLD-BLD-0001.md)  
**Fitting acceptance gate:** [CPLD-HW-0004](tasks/db/CPLD-HW-0004.md)

## Split of responsibilities

- `devel` contains the authoritative Git checkout, RTL source and combined Verilator testbench; the build driver runs here.
- `devel-rtl` contains Yosys, `~/Tools/atf15xx_yosys`, Wine and the Atmel `fit1508.exe` fitter. Its `~/rtl-build/src/` holds disposable, isolated remote run directories.
- `rtl_IOU_MMU_TB` remains exclusively a Verilator top. Physical CPLDs have *different* synthesis tops: `rtl_MMU/MMU.v` → `MMU` and `rtl_IOU/IOU.v` → `IOU`.
- The historical `rtl_MMU/{MMU.edif,MMU.fit,MMU.jed}` and `rtl_IOU/{IOU.edif,IOU.fit,IOU.jed}` are **not** transferred or used; do not interpret them as current-fit evidence.

## Prerequisites

From `devel`:

```bash
ssh -o BatchMode=yes devel-rtl 'hostname; yosys -V; wine --version'
command -v rsync
```

On `devel-rtl`, the upstream `atf15xx_yosys` checkout must contain `cells.lib`, `techmap.v`, `run_fitter.sh` and `vendor/{fit1508.exe,aprim.lib,atmel.std}`. The fitter's Wine prefix is `~/.wine_atf` as configured by the upstream helper. The default build target is **ATF1508ASV, PLCC84, 15 ns**, corresponding to `ATF1508ASV-15JU84`.

## Running

From any directory in the Apple CPLD checkout on `devel`:

```bash
./scripts/build-cpld.sh mmu
./scripts/build-cpld.sh iou
./scripts/build-cpld.sh all
```

Explicit build variants when required by the eventual pin/interface contract:

```bash
CPLD_MMU_DEFINES=FULL_MMU_ADDR ./scripts/build-cpld.sh mmu
CPLD_IOU_DEFINES=IOU_COMPLETE ./scripts/build-cpld.sh iou
CPLD_MMU_DEFINES=SPK_IN_MMU ./scripts/build-cpld.sh mmu
```

The default build has no extra defines. **Do not assume the default variant is the approved physical PCB configuration.** `FULL_MMU_ADDR` changes the MMU address pin count; `IOU_COMPLETE` changes IOU exported pins. Ticket HW-0004 must document which variants were fitted and approved. The driver does not set physical pin assignments or timing constraints; it preserves whatever pin annotations are present in the top module and reports what the fitter chooses.

`RTL_BUILD_HOST` can override `devel-rtl` with an SSH host alias. All other build environment/RTL tooling stays on `devel-rtl`.

## How it works

For *each* selected top, the driver creates an isolated remote `~/rtl-build/src/cpld.<TOP>.<random>/` and transfers Verilog files for that physical design. For the MMU, the source set also includes four shared helper modules currently stored under `rtl_IOU/`: `TTL_ls138_2.v`, `TTL_ls259.v`, `IOU_exp16.v` and `IOU_toggle_d.v` (the latter is used by the `SPK_IN_MMU` variant). These are *RTL dependencies*, not the IOU CPLD top: `IOU.v`, its SPI slave and the combined testbench are not transferred for an MMU build. Missing shared files cause an explicit failure. The script then streams `scripts/remote-fit-cpld.sh` over SSH. The remote helper creates `synth.ys` reading every module, explicitly selects the top, maps the Atmel logic primitives, generates EDIF using `write_edif -lsbidx`, and calls the upstream fitter with explicit `-d ATF1508ASV -p PLCC84 -s 15`.

Outputs are copied into unique ignored directories such as:

```text
build/rtl-cpld/MMU_20261121_171134/
    MMU.edif
    MMU.fit
    MMU.jed
    MMU.pin       # if generated
    MMU.io        # if generated
    MMU.tt3       # if generated
    synth.ys
    yosys.log
    fitter.log
    toolchain.txt
    sources.sha256
```

The equivalent IOU results use a distinct directory such as
`build/rtl-cpld/IOU_20261121_171134/`. Timestamps are generated on `devel`
using its **local clock and timezone**, in `YYYYMMDD_HHMMSS` format. The
created directory is exclusive: if a build with the same timestamp already
exists, an atomic `mkdir` retry appends `_01`, `_02`, etc., so earlier builds
are never overwritten. This applies to both success and failure output.

On a failing build, the client still collects diagnostics when available and exits nonzero. A generated `.jed` file alone does not authorize physical programming: examine the FIT resource, pin, JTAG/security, timing and warning data. Existing remote runs are intentionally retained for diagnosis; clean obsolete `~/rtl-build/src/cpld.*` directories manually after confirming artifact archival.

## Tests and acceptance

Run local infrastructure tests (fake SSH/rsync/Yosys/fitter; **not** a hardware fit):

```bash
bash -n scripts/build-cpld.sh scripts/remote-fit-cpld.sh tests/test_remote_build.sh
bash tests/test_remote_build.sh
```

After updating the working branch on `devel`, run `./scripts/build-cpld.sh all`. Inspect both `.fit` files and `toolchain.txt`, note source SHA checksums and compare pins/macrocells/timing with the physical design. Re-run relevant existing Verilator suites if any RTL is changed. Record the real outputs and results in HW-0004; do not mark that ticket RESOLVED merely because the driver passes mock tests.
