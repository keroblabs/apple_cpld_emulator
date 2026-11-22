# CPLD-BLD-0002 — Remote Quartus Prime Lite MAX II single-CPLD build workflow

**ID:** `CPLD-BLD-0002`
**Status:** `REVIEW`
**Title:** Remote Quartus Prime Lite MAX II single-CPLD build workflow
**Reporter:** Project owner (Kerob Labs)
**Reviewer:** Project owner; GPT-6
**Assignee:** GPT-6 (ChatGPT) — build-script and project-file implementation

**Description**

Add a reproducible, source-tracked Quartus Prime Lite 25.1 build backend for the owner-developed `rtl_single/cpld_top.v`, so `devel` remains the Git/source-of-truth workstation and `devel-rtl` performs MAX II synthesis/fitting/timing estimation without moving development source ownership. Target EPM240T100C5 compact first; make EPM240 full-address, EPM570T100C5 and EPM570T144C5 selectable. Keep the existing two-ATF1508 build flow unchanged.

**Details**

- Kind: TASK
- Category: `CPLD-BLD`
- Milestone: M1/M3 — single-CPLD MAX II feasibility
- Branch: `quartus-maxii-build`, created from `rtl-compile-work-single-cpld` after the owner's implementation and Codex fixes, on 2026-11-22.
- Dependencies: [CPLD-RTL-0001](CPLD-RTL-0001.md), installed Quartus Lite 25.1 with MAX II support on `devel-rtl`. Future hardware acceptance relates to [CPLD-HW-0006](CPLD-HW-0006.md).
- Toolchain installation verified by owner before this ticket: `quartus_sh`, `quartus_map`, `quartus_fit`, `quartus_sta`, `quartus_cpf` version 25.1std.0 build 1129; EPM240T100C5 and EPM570 variants listed in device database. This **does not prove compilation**.
- Source: only `rtl_single/*.v` and tracked `quartus/maxii` project inputs, with no stale fitter outputs.
- Expected results: `build/rtl-maxii/PART_VARIANT_YYYYMMDD_HHMMSS[_NN]/` containing effective project settings, logs, source hash list and available synthesis/fit/timing reports, including POF only upon full build success.
- Physical limitations: **no board-specific pinout, no fabricated I/O delay constraints, no timing/CDC sign-off, and no hardware programming authorization.** The sample SDC includes only the earlier simulated 1 MHz PHI_0/125 kHz SPI clock assumptions.

**Current Work**

| State | Work / evidence | Responsible | Next action |
| --- | --- | --- | --- |
| DONE | Tracked QPF/QSF/SDC plus Tcl configuration for part and compact/full variant | ASSIGNEE (GPT-6) | Owner review |
| DONE | `scripts/build-maxii.sh` sources from `devel`, isolates remote work, saves collision-safe timestamped results | ASSIGNEE (GPT-6) | Execute mocked and real tests |
| DONE | `scripts/remote-fit-maxii.sh` uses Quartus install on `devel-rtl`, runs full compile, retains reports on failure | ASSIGNEE (GPT-6) | Execute mocked and real tests |
| DONE | No-network fake SSH/rsync/Quartus test harness and user instructions supplied | ASSIGNEE (GPT-6) | Run tests on `devel` |
| PENDING | `bash -n`, mock script execution and inspection of any failing cases | ASSIGNEE / reviewer | Record exact results; not performed by ChatGPT |
| DONE (owner-reported) / REVIEW (timing and artifacts) | Owner supplied EPM240T100C5 compact FIT summary: successful fit, **223/240 LEs (93%)**, **64/80 I/O pins (80%)**, 0/1 UFM, final timing models; POF generated. Full FIT/MAP/STA logs, warnings, source hashes and pin plan still to be reviewed | Project owner; GPT-6 reviewer | Inspect `build/rtl-maxii/EPM240T100C5_compact_20261122_231057/` and consider EPM570 comparison |
| DONE (owner-reported fit comparison), PENDING (reports/traceability) | Both compact targets fitted: EPM240T100C5 **223/240 LEs, 64/80 pins**; EPM570T100C5 **223/570 LEs, 64/76 pins**. Register usage, full warnings, exact source hashes and STA review still pending | Project owner / reviewer | Inspect both detailed outputs before release or board pin freeze |
| DONE (owner-reported STA summary), PENDING (full timing qualification) | EPM570 PHI_0 Fmax 38.69 MHz; SPI_CLK Fmax 80.03 MHz; setup slack +487.077/+494.586 ns, setup TNS 0; hold slack +1.078/+1.645 ns, hold TNS 0. These are positive for paths analyzed under experimental clock constraints | Project owner / reviewer | Inspect complete STA report for unconstrained paths, cross-clock transfers, input/output delays; physical timing/CDC remains in HW-0006 |
| PENDING | Final physical board pinout, I/O constraints and CDC/voltage qualification | Project owner / hardware reviewer | Separate HW-0006 scope |
| PENDING | Owner acceptance and merge | Project owner | RESOLVED after reviewed evidence; CLOSED only on main merge |

**Notes**

- **2026-11-22 — REPORTER (Project owner) — Request:** After personally consolidating and testing the single-CPLD RTL, the owner requested scripts analogous to the existing ATF1508 flow, building a real Quartus project remotely on `devel-rtl` to determine whether an EPM240 can fit.
- **2026-11-22 — REVIEWER (GPT-6) — Source baseline:** Inspected new owner/Codex fixes in `rtl-compile-work-single-cpld`: `D7_status_en` is now a continuously driven `wire`; `cpld_top` references were corrected in the emulator header. Full behavioral verification remains an independent RTL ticket acceptance condition.
- **2026-11-22 — ASSIGNEE (GPT-6) — Implementation:** Created new `quartus/maxii/` project files `cpld_top.qpf`, `cpld_top.qsf`, `cpld_top.sdc`, `configure.tcl`; separate client/remote scripts `build-maxii.sh` and `remote-fit-maxii.sh`; `tests/test_remote_maxii_build.sh` and `docs/remote-maxii-build.md`. Existing `build-cpld.sh` / `remote-fit-cpld.sh` remain unmodified.
- **2026-11-22 — REVIEWER (GPT-6) — Verification boundary:** Project-file syntax was designed against Quartus Standard Tcl/command-line conventions and checked by source inspection, but **no executable Quartus/SSH connection to the owner's devel-rtl VM was available here**. Neither mock tests nor an EPM240 hardware fit are claimed to have run. Any first-run error must remain on this ticket in REVIEW until corrected.

- **2026-11-22 — REVIEWER (GPT-6) — Recorded owner-reported first real MAX II build:** Project owner executed `bash scripts/build-maxii.sh epm240` on `devel`. Console reported `Quartus built EPM240T100C5 (compact); timing and pinout NOT approved.` and `MAX II EPM240T100C5 (compact) reports and POF: /home/marcio/Projects/emulators/apple_cpld_emulator/build/rtl-maxii/EPM240T100C5_compact_20261122_231057`. The isolated remote build directory is `/home/marcio/rtl-build/src/maxii.cpld_top.rF4Fo5R6`. **Result: first full Quartus compilation and POF generation owner-reported successful**, subject to direct FIT/MAP/STA and source-hash review. The `bash -n` command emitted no errors. However, `bash tests/test_remote_maxii_build.sh` returned **no visible PASS lines**, although the checked-in mock script prints them; verify the test's exit status separately before claiming mock tests passed. No board-pin/timing sign-off or hardware programming is implied.

- **2026-11-22 — REVIEWER (GPT-6) — Recorded owner-supplied EPM240 fitter summary:** Owner pasted Quartus Prime Lite 25.1std.0 build 1129 `cpld_top` MAX II **EPM240T100C5** report with **Fitter Status: Successful — Sun Nov 22 23:11:06 2026**, **Timing Models: Final**, **Total logic elements: 223/240 (93%)**, **Total pins: 64/80 (80%)**, **Total virtual pins: 0**, **UFM blocks: 0/1 (0%)**. The summary demonstrates a successful owner-run fit in the reduced-I/O compact configuration. It leaves **17 LEs (7.1%)** and **16 user I/O pins** unallocated numerically. `Timing Models: Final` does *not* mean timing closure. The detailed timing report, timing constraints, warnings, pin mapping and actual build-source hashes have not been independently inspected; this ticket remains REVIEW. No additional fit/regression claims are made.

- **2026-11-22 — REVIEWER (GPT-6) — Recorded owner-reported EPM570 result:** Owner executed `bash scripts/build-maxii.sh epm570` on `devel`, and command reported success with POF under `build/rtl-maxii/EPM570T100C5_compact_20261122_231422/` (remote workspace `~/rtl-build/src/maxii.cpld_top.9Hdz2hRu`). Owner then pasted the fitter summary: **Successful — Sun Nov 22 23:14:31 2026**, Quartus **25.1std.0 Build 1129**, `cpld_top`, MAX II **EPM570T100C5**, **223/570 LEs (39%)**, **64/76 I/O pins (84%)**, **0 virtual pins**, **0/1 UFM**. Identical LE usage to the compact EPM240 fit gives **347 spare LEs** versus **17**, but only **12 spare physical I/O pins** versus **16**. This is a fit and build result, **not numerical timing closure or schematic pinout approval**. Source hash equality, full warnings and STA still require inspection.

- **2026-11-22 — REVIEWER (GPT-6) — Recorded owner-supplied EPM570 STA summaries:** The owner provided `Fmax Summary`: `PHI_0` **38.69 MHz** (restricted 38.69 MHz), `SPI_CLK` **80.03 MHz** (restricted 80.03 MHz). The owner subsequently provided `Setup Summary` **PHI_0 +487.077 ns, SPI_CLK +494.586 ns, endpoint TNS 0.000 for each**; `Hold Summary` **PHI_0 +1.078 ns, SPI_CLK +1.645 ns, endpoint TNS 0.000 for each**. Under the currently tracked illustrative SDC periods (PHI_0 1000 ns, SPI_CLK 8000 ns), the *reported constrained endpoints* show no setup or hold failures. This is a valuable initial numerical static-timing observation, **not full timing sign-off**, because complete path/exception/unconstrained reports, external I/O delay budget, physical pin mapping and asynchronous SPI/CPU CDC still need review. Detailed `.sta.rpt` file was not independently inspected.

**Proposed Change**

Use the tracked source-controlled project plus isolated remote helper to compile `cpld_top` using `quartus_sh -t configure.tcl PART VARIANT` and `quartus_sh --flow compile cpld_top`. Preserve `.map.rpt`, `.fit.rpt`, `.sta.rpt`, pin information, warnings, effective `.qsf`, tool versions and input SHA-256. Reject failed builds without leaving a stale POF. Default to a compact EPM240T100C5 build, and only after comparing the resulting fit select EPM570 or pursue further RTL optimization.

**Acceptance criteria**

- [ ] Client and remote scripts pass bash syntax checks and no-VM mock test harness; failures preserve logs and return nonzero.
- [ ] Real `devel-rtl` Quartus 25.1 full flow compiles a verified source tree or produces actionable fitter diagnostics with exact target.
- [ ] Effective QSF/SDC, input hashes, tool versions and map/fit/timing reports are saved in a timestamped result path.
- [ ] Reported logic usage and pin counts are based on fitter output, not ATF macrocell-to-LE estimates.
- [ ] No physical pin assignment, electrical, timing, flash-programming or actual-board acceptance is inferred from a generated POF.
- [ ] Owner approves integration into the active work branch; CLOSED only on merge into main.

**Commits**

| Branch | Commit | Review Status | Description |
| --- | --- | --- | --- |
| `quartus-maxii-build` | `7b5ccec`–`0a30208` | IN_REVIEW | Tracked Quartus project and Tcl configuration |
| `quartus-maxii-build` | `9a881e1`, `7b0ab66` | IN_REVIEW | Local client and remote synthesis helper |
| `quartus-maxii-build` | `e110bb3`, `02970c7` | IN_REVIEW | Mock harness and build documentation |

**Resolution Description**

Pending no-network script test and first real Quartus EPM240 attempt; code is prepared, but no fit or timing result has yet been observed.
