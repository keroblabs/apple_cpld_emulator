# CPLD-BLD-0001 — Remote ATF1508ASV build driver for MMU and IOU

**ID:** `CPLD-BLD-0001`  
**Status:** `REVIEW`  
**Title:** Remote ATF1508ASV build driver for MMU and IOU

**Description**

The authoritative Apple CPLD RTL source is developed on `devel`; Yosys 0.63, `atf15xx_yosys`, Wine and the Atmel fitter are on the dedicated `devel-rtl` build VM. Existing `rtl_IOU/build.sh`, `rtl_MMU/build.sh`, and `rtl_IOU_MMU_TB/build.sh` are local Verilator helpers and are intentionally excluded from Git. There is no tracked, reproducible remote path to synthesize the physical MMU and IOU independently and return reports/JEDEC files.

**Details**

- Kind: TASK
- Category: `CPLD-BLD`
- Milestone: M1/M3 — Hardware-targeted RTL build infrastructure
- Baseline: `main` inspected 2026-11-21; apply to the owner's current working branch only after reviewing local changes
- Dependencies: `CPLD-HW-0004` (fitter acceptance gate); working `ssh -o BatchMode=yes devel-rtl`, `rsync`, Yosys and `atf15xx_yosys` on the VM
- Scope: tracked remote-build scripts, mock regression tests, operator instructions, logs and artifact transfer; no RTL redesign or Verilator build changes

**Notes**

- **2026-11-21 — Discovery:** `rtl_IOU/` defines top `IOU`, `rtl_MMU/` defines top `MMU`; both have multiple module files. `rtl_IOU_MMU_TB/` is a combined Verilator testbench and must not be used as a physical CPLD top. The upstream `run_yosys.sh` reads only `<name>.v`, so directly invoking it without loading dependencies is insufficient.
- **2026-11-21 — Build environment:** `devel-rtl` is separately identified, reachable by passwordless SSH, and has successfully produced `.edif`, `.fit` and `.jed` for an upstream MMU smoke test. That is not yet proof that this project's revised MMU/IOU fit.

**Proposed Change**

1. Add `scripts/build-cpld.sh` with `mmu`, `iou`, and `all` subcommands. Transfer only the selected top-level directory's `.v` sources; do not synthesize the combined testbench or use stale `.edif` files.
2. Add `scripts/remote-fit-cpld.sh`, executing on `devel-rtl` with explicit `MMU`/`IOU` top, Yosys's Atmel/EDIF technology-mapping sequence, and the `ATF1508ASV`, `PLCC84`, speed `15` fitter arguments.
3. Keep `build/rtl-cpld/` outputs ignored by Git and separate per run. Return `.edif`, `.fit`, `.jed`, pin/timing outputs when produced, tool versions, source hashes and stdout/stderr logs. Preserve diagnostics and failure exit codes.
4. Permit explicitly declared per-target preprocessor defines, but do not silently select `FULL_MMU_ADDR` or `IOU_COMPLETE`; hardware build variant choices must be reviewed in `CPLD-HW-0004`.
5. Add tests using local fake SSH/rsync/Yosys/fitter to validate target separation, read-all-sources behaviour, correct options, output collection and fail-fast/error propagation.

**Acceptance criteria / required evidence**

- [x] Shell syntax valid, mock tests pass for `mmu`, `iou`, `all`, rejected arguments, failed SSH and failed fitter.
- [x] Test harness confirms separate MMU/IOU top-level source transfer and no historical EDIF/JED or testbench transfer; **real design builds pending**.
- [x] Mock fitter asserts `-d ATF1508ASV -p PLCC84 -s 15` for each target; real fitter report still pending.
- [x] Mock tests verify unique output directories, nonzero propagation and preserved logs for Yosys and fitter errors.
- [x] Owner ran both physical builds on `devel` against `devel-rtl`, reporting successful ATF1508ASV PLCC84 fit and JTAG/security settings.
- [ ] Detailed fit files, timing/clock constraints, pin assignments, warnings and exact build variants reviewed under `CPLD-HW-0004`.
- [ ] Owner reviews the implementation before marking RESOLVED. The `rtl-compile-work` branch is updated for the owner-requested `git pull`; no merge is performed.

**Implementation and verification log**

- **2026-11-21 — Implementation:** New tracked `scripts/build-cpld.sh` chooses top-level MMU/IOU independently, copies only `.v` from the matching directory, and runs new `scripts/remote-fit-cpld.sh` over SSH. Yosys reads all module files and maps EDIF through the verified `atf15xx_yosys` primitive flow. Independent local outputs retain logs and source/toolchain metadata; combined Verilator flow unchanged.
- **2026-11-21 — Tests (isolated harness):** `bash -n scripts/build-cpld.sh scripts/remote-fit-cpld.sh tests/test_remote_build.sh` completed successfully. `bash tests/test_remote_build.sh` passed **8/8 assertions groups**: MMU correct source/ASV parameters and stale-EDIF exclusion; IOU independent source and `IOU_COMPLETE`; MMU optional `FULL_MMU_ADDR`; `all` output separation; invalid inputs; SSH failure; Yosys failure with captured log; fitter failure with captured EDIF/log. No real Yosys, Wine, remote VM, CPLD fit or physical timing measurements were executed in this isolated harness.
- **2026-11-21 — Handoff:** Apply the unpushed local patch series to the owner's actual `devel` checkout. Then execute real `./scripts/build-cpld.sh all` and evaluate the resulting FIT and JED artefacts as evidence for `CPLD-HW-0004`. Maintain REVIEW until owner acceptance; do not claim physical CPLD readiness.

- **2026-11-21 — Real MMU compile failure (owner on `devel`):** The first `./scripts/build-cpld.sh all` passed 8/8 mock checks but failed during MMU Yosys hierarchy resolution: `TTL_ls138_2` in `MMU_io` was absent from the transferred sources. Diagnostics were recovered under `build/rtl-cpld/MMU.bTyxHUAm/`. IOU fitting was not attempted in that run.
- **2026-11-21 — Root cause and fix:** `rtl_MMU/MMU_io.v` depends on `TTL_ls138_2`, `TTL_ls259`, `IOU_exp16`, and optionally `IOU_toggle_d`, which are stored in `rtl_IOU/`. Updated the build-driver source manifest to include those four named helpers during MMU compilation only. Physical IOU top and SPI implementation remain excluded. No RTL changes.
- **2026-11-21 — Regression and reproducibility:** `bash -n scripts/build-cpld.sh scripts/remote-fit-cpld.sh tests/test_remote_build.sh` passed and `bash tests/test_remote_build.sh` passed 10/10 mock groups in the isolated build workspace, including missing-dependency rejection and `SPK_IN_MMU` coverage. The tests were not executed on `devel` after the change; real MMU and IOU Yosys/fitter runs remain pending.
- **2026-11-21 — Branch publication requested by owner:** Instead of applying local patch files, the owner explicitly requested committing the correction directly to `rtl-compile-work` to retrieve it by `git pull`. Code/test/docs commit `4a5c4ca` made from branch head `a7d7203`; this ticket-only commit records the handoff. No PR or merge.
- **2026-11-21 — Real devel→devel-rtl fitting (owner report):** After pulling the cross-directory MMU source fix, the owner supplied complete FIT utilisation summaries for both independently fitted physical tops. IOU: 108/128 macrocells, 83/128 flip-flops, 36/64 user I/O, 14 foldbacks, 112/128 nodes+FB/MCs, 10 cascades, 365 product terms; MMU: 42/128 macrocells, 17/128 flip-flops, 39/64 user I/O, 3 foldbacks, 42/128 nodes+FB/MCs, 3 cascades, 133 product terms. Both summaries report `$Device PLCC84 fits; JTAG ON; Secure OFF`. This is owner-executed build evidence, *not* an independently executed test here. Build run directories, define variants, detailed timing and pin files were not provided; the hardware feasibility review remains with [CPLD-HW-0004](CPLD-HW-0004.md).


**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- |
| `cpld-bld-0001-remote-rtl` | `7aa2d55` | UNREVIEWED | Ticket and index entry (local patch-series SHA; changes on application) |
| `cpld-bld-0001-remote-rtl` | `33068f6` | UNREVIEWED | Remote build driver, isolated tests and operator documentation (local patch-series SHA; changes on application) |
| `rtl-compile-work` | `4a5c4ca` | UNREVIEWED | MMU shared dependency transfer, 10-group regression suite, and documentation committed directly to requested branch |

**Resolution Description**

Real independent MMU/IOU fit reported successful by owner; pending owner review of this build automation and detailed fitter/timing acceptance under `CPLD-HW-0004`.
