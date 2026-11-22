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
| PENDING | First real EPM240T100C5 compact build and inspection of map/fit/STA/POF | Project owner | `bash scripts/build-maxii.sh epm240` |
| PENDING | Confirm source revision and compare EPM240 vs EPM570 only if needed | Project owner / reviewer | Record LE, registers, pin and timing evidence |
| PENDING | Final physical board pinout, I/O constraints and CDC/voltage qualification | Project owner / hardware reviewer | Separate HW-0006 scope |
| PENDING | Owner acceptance and merge | Project owner | RESOLVED after reviewed evidence; CLOSED only on main merge |

**Notes**

- **2026-11-22 — REPORTER (Project owner) — Request:** After personally consolidating and testing the single-CPLD RTL, the owner requested scripts analogous to the existing ATF1508 flow, building a real Quartus project remotely on `devel-rtl` to determine whether an EPM240 can fit.
- **2026-11-22 — REVIEWER (GPT-6) — Source baseline:** Inspected new owner/Codex fixes in `rtl-compile-work-single-cpld`: `D7_status_en` is now a continuously driven `wire`; `cpld_top` references were corrected in the emulator header. Full behavioral verification remains an independent RTL ticket acceptance condition.
- **2026-11-22 — ASSIGNEE (GPT-6) — Implementation:** Created new `quartus/maxii/` project files `cpld_top.qpf`, `cpld_top.qsf`, `cpld_top.sdc`, `configure.tcl`; separate client/remote scripts `build-maxii.sh` and `remote-fit-maxii.sh`; `tests/test_remote_maxii_build.sh` and `docs/remote-maxii-build.md`. Existing `build-cpld.sh` / `remote-fit-cpld.sh` remain unmodified.
- **2026-11-22 — REVIEWER (GPT-6) — Verification boundary:** Project-file syntax was designed against Quartus Standard Tcl/command-line conventions and checked by source inspection, but **no executable Quartus/SSH connection to the owner's devel-rtl VM was available here**. Neither mock tests nor an EPM240 hardware fit are claimed to have run. Any first-run error must remain on this ticket in REVIEW until corrected.

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
