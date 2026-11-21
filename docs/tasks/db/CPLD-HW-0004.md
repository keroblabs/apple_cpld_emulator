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
