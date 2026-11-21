# CPLD-HW-0004 — Move peripheral outputs to MMU; validate CPLD fit and timing

**ID:** `CPLD-HW-0004`  
**Status:** `IN_PROGRESS`  
**Title:** Move peripheral outputs to MMU; validate CPLD fit and timing
**Reporter:** Project owner (Kerob Labs)
**Reviewer:** Project owner; GPT-6
**Assignee:** UNASSIGNED (RTL developer to be nominated by project owner)

**Description**

The reviewed `CPLD-HW-0002` and `CPLD-HW-0003` Verilog/emulator work is functionally accepted. The owner has now produced current ATF1508ASV fitter reports for both MMU and IOU via the dedicated `devel-rtl` environment. **Both previously reported variants fit PLCC84**, but the owner now requires a different physical signal allocation: move `CAS_OUT`, `SPK_OUT` and `AN0..AN3` to the MMU, while retaining the compact IOU without `IOU_COMPLETE`. The MMU must be re-fitted with `SPK_IN_MMU` and six external outputs restored. These revised physical configurations and their timing, pin/clock assignments and electrical integration remain **unverified**. This ticket is the explicit **CPLD timing/resource and software build feasibility gate** before declaring the designs ready to program or use as physical CPLDs.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1/M3 — hardware-targeted RTL verification
- Baseline: `cpld-hw-0001-0003-tickets` at 2026-11-21
- Dependencies: [CPLD-HW-0002](CPLD-HW-0002.md), [CPLD-HW-0003](CPLD-HW-0003.md)
- **Re-entry trigger:** SATISFIED on 2026-11-21: Yosys, `atf15xx_yosys` and the Atmel fitter available in the build-only `devel-rtl` environment.
- Scope: verified transfer of speaker/cassette/annunciator ownership from IOU to MMU, matching combined Verilator/emulator wiring, actual `SPK_IN_MMU`/non-`IOU_COMPLETE` compilation, RTL regression tests, ATF1508ASV synthesis/fitting/resource/timing and pin-review reports. **No UART implementation, schematic/PCB design, STM32 firmware, physical electrical tests or hardware commissioning.**
- Approved physical intent: `MMU` provides `CAS_OUT`, `SPK_OUT`, `AN0`, `AN1`, `AN2`, `AN3`; `IOU` is built with `IOU_COMPLETE` disabled. `FULL_MMU_ADDR` remains an independently selected physical address variant and must not change silently.
- Legacy scope limitation: the owner's default IOU FIT already excluded `IOU_COMPLETE`. Consequently its reported 108/128 macrocells do **not** demonstrate a further reduction from this relocation; an internal IOU `AN3` may remain required for status reads.

**Current Work**

| State | Work / acceptance evidence | Responsible | Next action / reference |
| --- | --- | --- | --- |
| DONE | Existing default IOU and baseline MMU fit ATF1508ASV PLCC84; IOU full FIT/PIN/TT3 reviewed, JTAG ON and Secure OFF | Reporter (owner) test; Reviewer (GPT-6) | Retain as baseline only; not proof for expanded MMU |
| DONE | Architectural allocation approved: MMU owns CAS_OUT, SPK_OUT and AN0–AN3; no UART in this change | Reporter (owner) | Implement and verify without changing Apple-facing bus semantics |
| PENDING | Re-enable `SPK_IN_MMU` and expose six outputs from `rtl_MMU/MMU_io.v` through `rtl_MMU/MMU.v` | Assignee (UNASSIGNED) | Verify C020/C030 toggles and C058–C05F annunciator set/clear |
| PENDING | Update `rtl_IOU_MMU_TB/IOU_MMU_tb.v` so external outputs come from MMU_io, not IOU_common; leave internal IOU AN3 status path working | Assignee (UNASSIGNED) | Build combined Verilator model, run functional integration and output ownership tests |
| PENDING | Build physical IOU without `IOU_COMPLETE` and MMU with `SPK_IN_MMU` using reproducible define configuration, excluding `ADD_UART` | Assignee (UNASSIGNED) | Check ignored local build.sh variants; introduce tracked config if needed |
| PENDING | Re-run MMU and IOU Yosys/Atmel fits; compare new utilisation, pins, warnings, JTAG/security and timing with baseline | Assignee / Reviewer | Save timestamped output folders, FIT/PIN/TT3/JED, hashes and tool versions |
| BLOCKED | Hardware timing, physical pin mapping, DATA bus isolation and SPI↔PHI_0 CDC not yet accepted | Reviewer + project owner | Review measured/spec-based timing and intended PCB pins; bench test later |

**Historical note provenance (best-effort; original Notes are unchanged)**

| Historical note(s) | Recording evidence | Actor supported by evidence | Confidence / limitation |
| --- | --- | --- | --- |
| 2026-11-21 "Defer by owner" | Initial tracker commit `8ea2fc2`; note explicitly says "by owner" | Reporter: project owner requested deferral | Explicit in note; Git author alone cannot prove who wrote it |
| 2026-11-21 "Source of obligation", "Clock qualification" | Initial tracker commit `8ea2fc2` | Recorded under Git identity "Kerob Labs" | Underlying analyst/agent UNKNOWN from Git evidence alone |
| 2026-11-21 re-entry, IOU/MMU resource summaries | Branch commit `7e77ded` following owner-provided fitter report | Reporter: owner supplied metrics; Reviewer: GPT-6 summarized them | Direct conversation provenance; Git author is not independent evidence of reviewer |
| 2026-11-21 full IOU FIT/PIN/TT3 review | Branch commit `ebf32ea`, assistant-led inspection | Reviewer: GPT-6 | Supported by review conversation and commit contents; physical testing not performed |

**Notes**

- **2026-11-21 — Defer by owner:** Tool installation is underway. Both functional implementations and test regressions passed developer review and independent code review; owner manually ran the emulator with `PR#3` / `PR#0`. This deferment does **not** establish that MMU or IOU fits the intended CPLD.
- **2026-11-21 — Source of obligation:** The original [HW-0002](CPLD-HW-0002.md) and [HW-0003](CPLD-HW-0003.md) acceptance criteria explicitly requested current ATF1508ASV fitting. This ticket transfers those **unfulfilled** criteria; historical November 2025 fit output is not evidence for the 2026-11-21 changes.
- **2026-11-21 — Clock qualification:** The HW-0003 digital phase sweep tests PHI_0 at 1 MHz and SCK at 125 kHz with an 8 µs pending-request hold. It does not prove actual post-fit timing, higher SCK, clock-routing or setup/hold margins.

- **2026-11-21 — Re-entry / first real fitter evidence:** On `rtl-compile-work`, following cross-directory MMU helper dependency fix [CPLD-BLD-0001](CPLD-BLD-0001.md), the owner reported successful independent MMU and IOU Yosys/Atmel fitting. Both FIT excerpts state `$Device PLCC84 fits; JTAG ON; Secure OFF`. These results are owner-reported text from actual build runs; this reviewer has not inspected the complete FIT, PIN, JED or timing files, nor executed the fit independently.
- **2026-11-21 — IOU resource summary (owner-provided):** 108/128 (84.4%) macrocells, 83/128 flip-flops, 36/64 I/O pins, 14 foldbacks, 112/128 nodes+FB/macrocells (87.5%), 10 cascades, 365 product terms. LAB A: 79/80 product terms, 39/40 fan-in; LAB D and E: 39/40 fan-in each; LAB F: 16/16 macrocells. **Interpretation:** fit passes, but local placement/fan-in/product-term margins are tight. Future IOU features or alternate build defines could fail even though 20 macrocells are globally unallocated.
- **2026-11-21 — MMU resource summary (owner-provided):** 42/128 (32.8%) macrocells, 17/128 flip-flops, 39/64 I/O pins, 3 foldbacks, 42/128 nodes+FB/macrocells, 3 cascades, 133 product terms. **Interpretation:** substantial global resource headroom for MMU, subject to selected defines and full timing/pin constraints.
- **2026-11-21 — Remaining verification:** Actual per-run source hashes, `synth.ys`, toolchain metadata, selected `CPLD_MMU_DEFINES`/`CPLD_IOU_DEFINES`, all fitter warnings, detailed pin assignment, timing reports and SPI SCK ↔ PHI_0 clock/CDC margin were not included in the reported summary. Recheck intended physical configurations, especially `FULL_MMU_ADDR` / `IOU_COMPLETE` if either is required, before hardware acceptance. No electrical, PCB or programmer test has been performed.

- **2026-11-21 — Independent review of latest owner-uploaded IOU artifacts:** Reviewed complete `IOU.fit` (1,540 lines), `IOU.pin` (123 lines) and `IOU.tt3` (328 lines), corresponding to Atmel fitter **v1918** run at **17:22:25 on 2026-11-21**. Exact SHA-256 digests: `IOU.fit` `5329316ff8cb110694147e38549dfeccece0836b822e2772e0ef96629d8ef979`; `IOU.pin` `89a86263f2e0ce832e4123db21b41a1fc10964831f1785fc659d31f9802f789e`; `IOU.tt3` `fbbcb19cf15de1d4c9e31d1e2d59f38813420849a22a2130378ea729d1836f51`. The report explicitly targets `-tech ATF1508ASV -device PLCC84 -tpd 15`, ends `End fitter, Design FITS`, and confirms `$Device PLCC84 fits; JTAG ON; Secure OFF`. Current IOU uses 108/128 macrocells, 83/128 flip-flops, 36/64 user I/O, 14 foldbacks, 10 cascades, 365 PTs. Four early fitter passes report one placement and three grouping warnings; the final strategy succeeds after retry with `Preassign = IGNORE`. These **intermediate warnings are not a final fit failure**; physical pin assignment is not yet locked against a PCB schematic.
- **2026-11-21 — IOU pin verification:** The FIT and PIN files agree for all 39 mapped external entries, including JTAG. `IOU.tt3` declares 35 non-JTAG user signal pins consistent with those assignments. `PHI_0` is placed at pin **2** (GCLK2-capable), `spi_clk` at **83** (GCLK1-capable), and `CPU_RW` at dedicated input **84**; `spi_cs` 74, `spi_mosi` 76, `spi_miso` 24, `RESET` 8, `DATA_EN_N` 50; JTAG pins TDI 14, TMS 23, TCK 62, TDO 71. Global-clock pins match the ATF1508ASV PLCC84 manufacturer pin table.
- **2026-11-21 — Clock-implementation findings:** Analysed 83 register clock equations in `IOU.fit`: **40** register clocks from `spi_clk`, **20** from `PHI_0`, **10** from `!PHI_0`, and **13** from address/control-qualified expressions involving `!PHI_0`, `C0XX_N`, `ADDR`, and sometimes `CPU_RW`. The generated/decoded clocks are structurally significant and warrant clock-glitch/setup/hold and bus-phase review. The design also crosses SPI clock and PHI_0 domains. The file does **not** include a numerical static-timing critical-path/slack/max-SCK report: `TPD = 15` is the requested speed grade, not measured path closure. The previous digital phase sweep is not a substitute for physical timing evidence.
- **2026-11-21 — Physical variant and bus enable qualification:** The `IOU.tt3` `#$ PINS` declaration lacks `CAS_OUT`, `SPK_OUT`, `AN0..AN3`, `SW2`, `PDL2`, and `PDL3`; therefore this is a **default IOU build without `IOU_COMPLETE`**. If the board requires these signals, re-fit that variant separately; the current 108/128 fit does not certify it. All eight `DATA_0..7.OE` equations are `CPU_RW` alone; `DATA_EN_N` is separate (pin 50). This may be intentional with an external bus transceiver controlled by `DATA_EN_N`, but the physical bus isolation must be verified to rule out contention during unrelated CPU read cycles. No RTL change authorised from these findings alone.
- **2026-11-21 — File-based review tests:** Ran a reproducible Python/regex assertion set on the three uploaded files: **10/10 PASS**, covering target/grade, final fitter success and JTAG/security, ten key pin mappings across FIT/PIN, seven non-dedicated pins cross-checked with TT3, all four JTAG pins, absence of `IOU_COMPLETE` pins, 83 register clock equations and 13 decoded clocks, eight DATA OE equations, warning/final-success distinction and absence of numerical STA metrics. No new RTL or toolchain build was run during this document review; file hashes are recorded above for traceability.
- **2026-11-21 — Remaining gates after full IOU file review:** (1) Owner identifies intended physical IOU build defines and tests `IOU_COMPLETE` if required; (2) physical pin plan must be frozen and matched to .pin/fitter output; (3) evaluate gated clocks and SPI↔PHI_0 asynchronous crossings against the required SCK frequency and data sampling; (4) confirm external DATA isolation; (5) inspect MMU's own full FIT/PIN evidence. Keep this ticket **IN_PROGRESS**, not RESOLVED.

- **2026-11-21 — REPORTER (Project owner) — Approved scope:** Move external speaker (`SPK_OUT`), cassette (`CAS_OUT`) and four annunciators (`AN0..AN3`) to the underutilised MMU. Build the IOU without `IOU_COMPLETE`, enable `SPK_IN_MMU` on MMU and in combined Verilator wiring. Do not enable `ADD_UART` at this stage; spare MMU capacity may support a future, separate UART ticket.
- **2026-11-21 — REVIEWER (GPT-6) — RTL investigation:** In `rtl_MMU/MMU_io.v`, speaker/cassette toggles already exist under `SPK_IN_MMU`, but annunciators currently terminate at `AN0_unused..AN3_unused`; the top `rtl_MMU/MMU.v` has commented-out annunciator ports. The combined `rtl_IOU_MMU_TB/IOU_MMU_tb.v` drives external `CAS_OUT`, `SPK_OUT`, `AN0..AN3` from `IOU_common`. Revise the connection source to `MMU_io`, preserving SPI/IOMEM and C0Fx interfaces. `IOU_common` still has an internal `AN3` state feeding `IOU_iomem` status readback; do not silently remove it without a separate semantics test or a justified inter-CPLD signal.
- **2026-11-21 — REVIEWER (GPT-6) — Build-config qualification:** Existing physical `IOU.fit` proves the **default, non-IOU_COMPLETE** variant, not the expanded variant. The previous `MMU.fit` did not include the requested `SPK_IN_MMU` external signals. Actual variant commands must include `CPLD_MMU_DEFINES=SPK_IN_MMU ./scripts/build-cpld.sh mmu` and `CPLD_IOU_DEFINES='' ./scripts/build-cpld.sh iou`; check `rtl_IOU_MMU_TB/build.sh`, which is ignored by Git and may still supply `IOU_COMPLETE`. The final build configuration must be reproducible in tracked files. `FULL_MMU_ADDR` remains a separate pin-count decision.
- **2026-11-21 — REVIEWER (GPT-6) — Verification plan:** Require a failing-before/passing-after Verilator regression for the C020/C030 toggle outputs, C058/C059 through C05E/C05F annunciator writes, the IOU AN3 status mirror, and no duplicate external IOU outputs. Rebuild combined emulator and run existing SPI/MMU/ROM-bus regression tests. Re-run both target fits and inspect actual post-relocation FIT/PIN/timing reports; do not inherit the prior MMU's 42/128 utilisation or claim the old IOU results reflect new interface wiring.

**Proposed Change**

**Approved implementation to perform before final fitting**

1. Re-enable `SPK_IN_MMU` paths in `MMU_io.v` and `MMU.v`, exposing `CAS_OUT`, `SPK_OUT`, `AN0..AN3` as physical outputs. Reconnect `TTL_ls259` Q4–Q7 to annunciator ports. No UART, no new CPU-visible protocol, no SPI change.
2. In the combined testbench, source the six external outputs from `MMU_io` and remove their external wiring from `IOU_common`; retain `IOU_common.AN3` internally for `IOU_iomem` status semantics until an independently proven replacement exists. Preserve the emulator's `rtl_interconnect_capture` speaker/cassette observation.
3. Make build variants explicit and reproducible: MMU `SPK_IN_MMU`, IOU **without** `IOU_COMPLETE`, optional `FULL_MMU_ADDR` considered separately. Check the untracked Verilator build helper; tracked tests/configuration must enforce the intended variant and reject accidental mismatches.
4. Add regressions for speaker/cassette toggles, annunciator control addresses `$C058–$C05F` (clear/set pairs), internal IOU `AN3` status behaviour, and unchanged MMU/address/SPI/ROM-bus behaviour. Validate relevant reset and PHI_0 edges.
5. Re-fit both physical tops for the **new** variants, save timestamped FIT/PIN/TT3/JED and toolchain/source-hash evidence. Review macrocell/product-term headroom, clock assignments, warnings, timing constraints and external pin plan. Do not conclude ready for PCB solely from a nominal fit.

**Existing fit/timing verification requirements (still applicable)**

1. **Reproducible toolchain:** Record `atf15xx_yosys`/Yosys/fitter versions, exact target (`ATF1508ASV-15JU84` if that is the device selected for each CPLD), synthesis flags, top module and constraints; run the current committed branch source.
2. **MMU fit:** Incorporate both shared `BUS_RD_N`/`BUS_WR_N` outputs plus all existing control pins. Verify I/O count, macrocells, combinational logic and fitter timing; inspect mutually exclusive strobes and any output/clock pin constraints.
3. **IOU fit:** Incorporate the full 13-bit SPI receiver, five-bit selector decode and saturated counter. Record macrocell, I/O and clock/timing results and compare with the previous revision where feasible.
4. **Report and disposition:** Attach commands, tool output, final fit summaries and any limitations. If fitting or timing fails, document the exact violation and create a focused revised RTL change for review. Do not silently delete features to make the fitter pass.
5. **Release gate:** Classify project as **Verilator/emulator verified and CPLD fitter-fit proven for the reported builds; timing, variant, pin/clock and physical validation pending**, irrespective of PR merge status.

**Acceptance criteria / required evidence**

- [ ] MMU exposes speaker, cassette and all four annunciators, and combined Verilator external outputs are driven by MMU instead of IOU.
- [ ] IOU is built without `IOU_COMPLETE`; MMU is built with `SPK_IN_MMU`; UART remains disabled; intended `FULL_MMU_ADDR` variant is recorded.
- [ ] New output ownership verified with regression tests for `$C020`, `$C030`, and `$C058–$C05F`, and IOU status/SPI/mmu integration regressions pass.
- [ ] **Post-change** MMU and IOU synthesized and fitted for the approved physical variants, with updated macros, pins, warnings, JTAG/security and timing reviewed.
- [ ] Intended ATF1508ASV device/package, top modules, build definitions and exact toolchain documented.
- [x] **Previous, pre-reallocation** MMU variant synthesized and fitted on ATF1508ASV PLCC84; baseline resources summarized by owner. `BUS_RD_N`/`BUS_WR_N` are present in reviewed RTL. **Post-change re-fit required.**
- [x] **Previously fitted default (non-IOU_COMPLETE)** IOU on ATF1508ASV PLCC84; baseline resources summarized by owner and detailed review accepted as evidence. Recheck actual final paired variant and regressions.
- [ ] Source hashes, exact define variants and full toolchain identification checked against intended physical configurations.
- [ ] Pin assignments, timing/clock constraints, detailed warnings and resource margin reviewed; repeat relevant regressions if RTL changes are made.
- [x] No physical board design or bench verification claimed.
- [ ] Owner/reviewer accepts the fit evidence before marking **RESOLVED**.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `rtl-compile-work` | *documentation handoff* | UNREVIEWED | Owner-reported independent MMU/IOU FIT summaries; exact documentation commit recorded in branch history |

**Resolution Description**

**IN_PROGRESS:** The owner approved reallocation of six external outputs from IOU to MMU. The previous default IOU and MMU fit results are only baselines; the new MMU `SPK_IN_MMU` variant and corresponding combined emulator wiring are **not yet implemented or fitted**. Regression tests, pin/timing analysis, reviewer acceptance, and full hardware/PCB electrical work remain pending. No UART is included.
