# CPLD-RTL-0001 — Consolidate MMU and IOU into one synthesizable CPLD RTL

**ID:** `CPLD-RTL-0001`
**Status:** `REVIEW`
**Title:** Consolidate MMU and IOU into one synthesizable CPLD RTL
**Reporter:** Project owner (Kerob Labs)
**Reviewer:** GPT-6; project owner (final acceptance)
**Assignee:** Codex (GPT-6) — assigned for RTL1-R3/R5 corrective work

**Description**

Implement a single physical-CPLD RTL hierarchy for the Apple CPLD Emulator, replacing the two-chip MMU/IOU arrangement with a combined `cpld_top`. Share memory/soft-switch/expansion-control logic that was previously replicated or partitioned between separate CPLDs, and retarget the existing Verilator emulator without rewriting its bus state machines. The intended devices are **EPM240** (preferred feasibility target) and **EPM570** (alternative); successful fit, timing closure and physical design are **not yet demonstrated**. This ticket retrospectively registers owner-performed work in two phases.

**Details**

- Kind: TASK
- Category: `CPLD-RTL` (new RTL-consolidation category)
- Milestone: M1/M3 — Single-CPLD architecture / synthesizability
- Baseline: `main` at `572b0607`; review of `rtl-compile-work` (`d4da93b`) and `rtl-compile-work-single-cpld` (`3c2a5ca`) on 2026-11-22.
- Original phase 1 and phase 2 implementation executor: **Project owner (Kerob Labs)**, who personally implemented both phases; this remains distinct from the current RTL1-R3/R5 corrective assignment.
- Related work: [CPLD-HW-0004](CPLD-HW-0004.md) (prior separate-CPLD fitting), [CPLD-HW-0006](CPLD-HW-0006.md) (hardware timing, pin/electrical qualification), [CPLD-HW-0005](CPLD-HW-0005.md) (flash-update grant).
- Phase 1, [`rtl-compile-work`](https://github.com/keroblabs/apple_cpld_emulator/tree/rtl-compile-work): owner created `rtl_single/cpld_top.v` and a reorganized self-contained set of RTL modules derived from `rtl_IOU_MMU_TB/IOU_MMU_tb.v`, simplifying internal ownership of shared IOU/MMU functions; `rtl_single/build.sh` added for Verilator.
- Phase 2, [`rtl-compile-work-single-cpld`](https://github.com/keroblabs/apple_cpld_emulator/tree/rtl-compile-work-single-cpld): owner switched CMake's Verilator library/object path to `Vcpld_top` / `rtl_single/obj_dir`, updated `cpld/rtl_interface.h`, and stopped driving omitted inputs in `cpld/rtl_model.cpp`.
- Interface intent: `SW2`, `PDL2`, `PDL3` were **intentionally removed** from the external interface. In `rtl_single/io_common.v`, the corresponding C06x reads are tied to `1'b0`. Preserve this limitation explicitly; restoration or alternative multiplexing is future work requiring owner approval.
- Source-level external-port estimate: **64 signals** with compact 5-bit `MEM_ADDR_OUT`; **76 signals** with `FULL_MMU_ADDR` (17-bit). These are logical port counts including the bidirectional 8-bit DATA bus, **not physical-fit evidence**. The physical device/board pin allocation, voltage, global-clock, JTAG and timing constraints remain unverified.
- Scope exclusions: no physical PCB sign-off, no MAX II Quartus project/QSF/SDC, no new SPI/IOMEM protocol, no change to the authorized NOR update architecture, and no implementation of omitted paddles/button.

**Current Work**

| State | Work / acceptance evidence | Responsible | Next action / reference |
| --- | --- | --- | --- |
| DONE | Owner created consolidated RTL hierarchy in `rtl_single/` and integrated shared soft-switch/EXP16 signals | ASSIGNEE (owner) | Phase 1 commit `d4da93b` |
| DONE | Owner switched Verilator/CMake integration to `Vcpld_top` and removed SW2, PDL2, PDL3 writes | ASSIGNEE (owner) | Phase 2 commit `3c2a5ca` |
| DONE (owner-reported) | Emulator works with `PR#3` and `PR#0`; no other post-switch test performed | ASSIGNEE (owner) | No test log or independent execution supplied |
| DONE | Reviewer inspected combined top, shared blocks and the three-file phase-2 integration diff | REVIEWER (GPT-6) | Findings RTL1-R1 through R5 below |
| PENDING | Independent Verilator build and relevant MMU/ROM-bus/SPI functional regressions against both combined designs, with explicit feature coverage for omitted inputs | ASSIGNEE (owner) / reviewer | Record command, variant, test count, result and source commit |
| DONE | Changed `D7_status_en` from an initialized `reg` to a continuously driven `wire` declaration assignment | ASSIGNEE (Codex/GPT-6) | RTL1-R3 source correction; see Notes |
| DONE | Updated the `cpld/rtl_interface.h` interface comment and `SPLIT_CPLD` diagnostic to name `cpld_top` | ASSIGNEE (Codex/GPT-6) | RTL1-R5 maintenance; see Notes |
| PENDING | Verify dynamic C01x/C06x/DHIRES status gating with focused simulation/synthesis regression | Project owner / reviewer | RTL1-R3; record exact command and result |
| DONE (owner-reported fit), PENDING (timing/traceability) | EPM240T100C5 compact fit successful: 223/240 LEs (93%), 64/80 pins (80%); source hashes, warnings, pin plan and SDC/STA interpretation outstanding | ASSIGNEE (owner); reviewer (GPT-6) | [CPLD-BLD-0002](CPLD-BLD-0002.md); review detailed reports, optional EPM570 comparison |
| PENDING | Clock/I/O constraints, report of unconstrained paths, timing and CDC review before physical acceptance | Hardware reviewer / owner | Related HW-0006; do not interpret fit alone as timing pass |
| PENDING | Owner-controlled review/PR/merge to main | Reporter (owner) | Resolve only with accepted evidence; close only after merge |

**Notes**

- **2026-11-22 — REVIEWER (GPT-6) — Recorded request:** Project owner requested a retrospective `CPLD-RTL-0001` ticket. Owner explicitly stated that **he was the assignee and implemented the work himself**, in two phases: `rtl-compile-work` for the new RTL and `rtl-compile-work-single-cpld` for switching the emulator.
- **2026-11-22 — REVIEWER (GPT-6) — Recorded owner test:** Project owner reports that `PR#3` and `PR#0` worked after the switch and states that **no further test was performed**. This is owner-reported successful operation, **not an independently executed test**, exhaustive regression, or Quartus fit.
- **2026-11-22 — REVIEWER (GPT-6) — Source inspection:** Compared `main..rtl-compile-work`, `rtl-compile-work..rtl-compile-work-single-cpld`, inspected `rtl_single/cpld_top.v`, `io_common.v`, `iomem_block.v`, `spi_iface.v`, `mmu_addr.v`, memory map/select, address16, slot and EXP16 modules, and the modified emulator integration files. The second phase is narrowly scoped to `cmake/CMakeLists.txt`, `cpld/rtl_interface.h`, `cpld/rtl_model.cpp`.
- **2026-11-22 — REVIEWER (GPT-6) — RTL1-R1 (POSITIVE):** Consolidated top connects the MMU and IOU soft switches/EXP16 state internally rather than retaining two physical CPLD packages. The emulator uses the `Vcpld_top` generated class and corresponding object directory. No obvious miswired top-level MMU/IOU control signal was found during this source inspection. This is not equivalence proof.
- **2026-11-22 — REVIEWER (GPT-6) — RTL1-R2 (DOCUMENTED FUNCTIONAL LIMITATION):** `SW2`, `PDL2`, and `PDL3` have been deliberately removed; C06x corresponding inputs are tied low. This may be acceptable for the reduced-I/O initial board, but it **does not preserve those three physical inputs**. Record reduced feature support in hardware and verification requirements; don't silently reinstate or claim it is identical to the original full-IOU configuration.
- **2026-11-22 — REVIEWER (GPT-6) — RTL1-R3 (POTENTIAL INHERITED BUG; VERIFICATION REQUIRED):** `rtl_single/iomem_block.v` contains `reg D7_status_en = ~(C01X_N & C06X_N & DHIRES_state_n);`. A declaration initialization is not a continuously updating Boolean assignment. This line is also present in the preexisting `rtl_IOU/IOU_iomem.v`, so it is **not introduced by this consolidation**. A stale/unknown enable could suppress status reads or cause tool-dependent synthesis behavior. Confirm by focused simulation/synthesis, then use a continuous `wire` assignment or combinational logic if required, with a regression test.
- **2026-11-22 — REVIEWER (GPT-6) — RTL1-R4 (BUILD/TIMING GAP):** `rtl_single/build.sh` is a Verilator compilation helper, not a Quartus build project. MAX II resource fit, pinout, dual-clock SCK/PHI_0 behavior and timing have not been exercised on EPM240/EPM570. Keep in REVIEW pending actual fitter evidence. Related physical qualification remains HW-0006.
- **2026-11-22 — REVIEWER (GPT-6) — RTL1-R5 (MINOR MAINTENANCE):** `cpld/rtl_interface.h` comments and the `SPLIT_CPLD` diagnostic still name `IOU_MMU_tb`; update documentation/diagnostics to `cpld_top` when tidying, without changing functional behavior.
- **2026-11-22 — ASSIGNEE (Codex/GPT-6) — Assignee handoff:** Per project-owner instruction for this work, responsibility for the RTL1-R3/R5 corrective actions transfers from Project owner (Kerob Labs) to Codex (GPT-6). The original phase 1 and phase 2 implementation remains owner-performed as recorded above; owner/reviewer verification items remain assigned in Current Work.
- **2026-11-22 — ASSIGNEE (Codex/GPT-6) — RTL1-R3 source correction:** Changed `D7_status_en` in `rtl_single/iomem_block.v` from an initialized `reg` to a continuously driven `wire` declaration assignment. This addresses the reviewed declaration-initialization issue. Focused behavioral simulation/synthesis verification was not run and remains pending in Current Work.
- **2026-11-22 — ASSIGNEE (Codex/GPT-6) — RTL1-R5 maintenance:** Updated the `cpld/rtl_interface.h` interface comment and `SPLIT_CPLD` diagnostic to identify the combined `cpld_top` RTL/model. This is comment/diagnostic-only maintenance; no functional behavior changed and no build/test was run.


- **2026-11-22 — REVIEWER (GPT-6) — Follow-up source review:** Inspected the owner's latest `rtl-compile-work-single-cpld` branch after the corrective commits. `rtl_single/iomem_block.v` now uses `wire D7_status_en = ~(C01X_N & C06X_N & DHIRES_state_n);`, supplying the continuous combinational status enable required by RTL1-R3. The `cpld/rtl_interface.h` active comment and `SPLIT_CPLD` error now name `cpld_top`, satisfying RTL1-R5. **Both changes are accepted at source level**, without claiming execution of focused status-read regressions or a real MAX II fit. The status remains REVIEW.
- **2026-11-22 — REVIEWER (GPT-6) — MAX II build workstream:** A separate branch, `quartus-maxii-build`, implements the new source-tracked remote Quartus compilation flow under [CPLD-BLD-0002](CPLD-BLD-0002.md). Source implementation here remains separate from build-tool acceptance and PCB/electrical acceptance.

- **2026-11-22 — REVIEWER (GPT-6) — Single-CPLD feasibility outcome:** Project owner supplied Quartus Fitter summary for MAX II **EPM240T100C5** compact interface; **fitting successful at 223/240 (93%) logic elements and 64/80 (80%) I/O pins**, with 0/1 UFM blocks. This **demonstrates owner-reported physical device resource fitting** of the combined RTL at the chosen unconstrained board pinout, but leaves only 17 LEs. It is not evidence of timing/CDC or physical-board validation; numeric area should not be extrapolated from ATF1508 macrocells. Keep REVIEW pending focused RTL regressions and numerical timing/constraints assessment; compare EPM570 if design-growth margin is important. Build details cross-referenced in [CPLD-BLD-0002](CPLD-BLD-0002.md).

**Proposed Change**

Accept the two-phase owner implementation as the **baseline** for the single-CPLD experiment. Retain the functional hierarchy and the emulator's bus/scheduler architecture. Perform focused verification of inherited status gating and explicit C06x reduced-input behavior; run regression comparisons with the previous combined design; then build target-specific Quartus projects for EPM240T100C5 and (if necessary) EPM570T100C5/T144C5. Report **actual** logic-element usage, registers, I/O usage, warnings, fitter success/failure, and timing after SDC constraints. Keep future paddle-button recovery separate, and keep flash-update/board-ready scope in existing HW tickets.

**Acceptance criteria**

- [ ] Phase 1 and phase 2 integrated Verilator build reproducible at recorded SHAs; full relevant emulator/MMU/SPI regressions or clearly documented departures.
- [ ] Status reads and reduced C06x mappings verified with targeted tests; inherited `D7_status_en` risk assessed and corrected if confirmed.
- [ ] EPM240/EPM570 synthesis/fit evidence recorded, including exact pin, area, build defines and tool version; failing smaller fit is acceptable if the architecture works on a documented fallback.
- [ ] Timing/CDC limitations and hardware qualification correctly separated into HW-0006.
- [ ] Reviewer and project owner accept the change before RESOLVED; owner merger to `main` before CLOSED.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `rtl-compile-work` | [`d4da93b`](https://github.com/keroblabs/apple_cpld_emulator/commit/d4da93b397d3031cd35b77e14ab678e5274f6ec5) | IN_REVIEW | Owner's phase 1: add reorganized single-CPLD RTL modules in `rtl_single/` |
| `rtl-compile-work-single-cpld` | [`3c2a5ca`](https://github.com/keroblabs/apple_cpld_emulator/commit/3c2a5ca01440e7dad14297cbf3884452974a9d0b) | IN_REVIEW | Owner's phase 2: retarget emulator and CMake to `Vcpld_top` |
| `rtl-compile-work-single-cpld` | `fa79081` | APPROVED | RTL1-R3: change `D7_status_en` to a continuously driven wire; focused behavioral verification remains pending |
| `rtl-compile-work-single-cpld` | `e29617d` | APPROVED | RTL1-R5: update the `cpld_top` interface comment and `SPLIT_CPLD` diagnostic |

**Resolution Description**

Pending. Source architecture is promising and the owner reports a successful PR#3/PR#0 smoke test; no independent full regression, MAX II fitting, timing report or physical board validation has yet established final acceptance.
