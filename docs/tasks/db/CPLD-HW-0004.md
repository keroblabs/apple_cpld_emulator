# CPLD-HW-0004 — Validate MMU/IOU fitting and timing on ATF1508ASV

**ID:** `CPLD-HW-0004`  
**Status:** `IN_PROGRESS`  
**Title:** Validate MMU/IOU fitting and timing on ATF1508ASV

**Description**

The reviewed `CPLD-HW-0002` and `CPLD-HW-0003` Verilog/emulator work is functionally accepted. The owner has now produced current ATF1508ASV fitter reports for both MMU and IOU via the dedicated `devel-rtl` environment. **Both fit PLCC84**, but full timing, pin/clock assignments, selected build variants and physical-electrical validation remain open. This ticket is the explicit **CPLD timing/resource and software build feasibility gate** before declaring the designs ready to program or use as physical CPLDs.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1/M3 — hardware-targeted RTL verification
- Baseline: `cpld-hw-0001-0003-tickets` at 2026-11-21
- Dependencies: [CPLD-HW-0002](CPLD-HW-0002.md), [CPLD-HW-0003](CPLD-HW-0003.md)
- **Re-entry trigger:** SATISFIED on 2026-11-21: Yosys, `atf15xx_yosys` and the Atmel fitter available in the build-only `devel-rtl` environment.
- Scope: RTL synthesis, ATF1508ASV fitting, utilisation and timing reports; optional minimal RTL fixes with separate review. **No schematics, PCB work, STM32 firmware, physical electrical tests or hardware commissioning.**

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

**Proposed Change**

1. **Reproducible toolchain:** Record `atf15xx_yosys`/Yosys/fitter versions, exact target (`ATF1508ASV-15JU84` if that is the device selected for each CPLD), synthesis flags, top module and constraints; run the current committed branch source.
2. **MMU fit:** Incorporate both shared `BUS_RD_N`/`BUS_WR_N` outputs plus all existing control pins. Verify I/O count, macrocells, combinational logic and fitter timing; inspect mutually exclusive strobes and any output/clock pin constraints.
3. **IOU fit:** Incorporate the full 13-bit SPI receiver, five-bit selector decode and saturated counter. Record macrocell, I/O and clock/timing results and compare with the previous revision where feasible.
4. **Report and disposition:** Attach commands, tool output, final fit summaries and any limitations. If fitting or timing fails, document the exact violation and create a focused revised RTL change for review. Do not silently delete features to make the fitter pass.
5. **Release gate:** Classify project as **Verilator/emulator verified and CPLD fitter-fit proven for the reported builds; timing, variant, pin/clock and physical validation pending**, irrespective of PR merge status.

**Acceptance criteria / required evidence**

- [ ] Intended ATF1508ASV device/package, top modules, build definitions and exact toolchain documented.
- [x] MMU synthesized and fitted on ATF1508ASV PLCC84; resource and I/O utilisation summarized by owner. `BUS_RD_N`/`BUS_WR_N` are present in reviewed RTL.
- [x] IOU synthesized and fitted on ATF1508ASV PLCC84; resource and I/O utilisation summarized by owner. Reviewed SPI fixes are present in RTL.
- [ ] Source hashes, exact define variants and full toolchain identification checked against intended physical configurations.
- [ ] Pin assignments, timing/clock constraints, detailed warnings and resource margin reviewed; repeat relevant regressions if RTL changes are made.
- [x] No physical board design or bench verification claimed.
- [ ] Owner/reviewer accepts the fit evidence before marking **RESOLVED**.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `rtl-compile-work` | *documentation handoff* | UNREVIEWED | Owner-reported independent MMU/IOU FIT summaries; exact documentation commit recorded in branch history |

**Resolution Description**

**IN_PROGRESS:** Both designs report a successful ASV PLCC84 fit with JTAG enabled and security off. Resource utilisation and IOU local congestion are documented. Detailed timing, pin/clock constraints, exact build variant/toolchain evidence and owner review remain before RESOLVED. Physical validation is outside ticket scope.
