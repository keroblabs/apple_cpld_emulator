# CPLD-HW-0004 — Fit reviewed MMU/IOU RTL to ATF1508ASV when tools are ready

**ID:** `CPLD-HW-0004`  
**Status:** `DEFERRED`  
**Title:** Fit reviewed MMU/IOU RTL to ATF1508ASV when tools are ready

**Description**

The reviewed `CPLD-HW-0002` and `CPLD-HW-0003` Verilog/emulator work is functionally accepted, but **neither revised CPLD has a current ATF1508ASV synthesis/fitter/resource/timing report**. The owner's `atf15xx_yosys` installation on the dedicated RTL development environment is ongoing. **Do not block software acceptance or an owner-controlled PR on tool availability.** Carry this as a separate, explicit **software build and CPLD feasibility gate** before declaring the designs ready to program or use as physical CPLDs.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1/M3 — hardware-targeted RTL verification
- Baseline: `cpld-hw-0001-0003-tickets` at 2026-11-21
- Dependencies: [CPLD-HW-0002](CPLD-HW-0002.md), [CPLD-HW-0003](CPLD-HW-0003.md)
- **Re-entry trigger:** `atf15xx_yosys`, the ATF15xx fitter and necessary target device support become available in the build-only `devel-rtl` environment.
- Scope: RTL synthesis, ATF1508ASV fitting, utilisation and timing reports; optional minimal RTL fixes with separate review. **No schematics, PCB work, STM32 firmware, physical electrical tests or hardware commissioning.**

**Notes**

- **2026-11-21 — Defer by owner:** Tool installation is underway. Both functional implementations and test regressions passed developer review and independent code review; owner manually ran the emulator with `PR#3` / `PR#0`. This deferment does **not** establish that MMU or IOU fits the intended CPLD.
- **2026-11-21 — Source of obligation:** The original [HW-0002](CPLD-HW-0002.md) and [HW-0003](CPLD-HW-0003.md) acceptance criteria explicitly requested current ATF1508ASV fitting. This ticket transfers those **unfulfilled** criteria; historical November 2025 fit output is not evidence for the 2026-11-21 changes.
- **2026-11-21 — Clock qualification:** The HW-0003 digital phase sweep tests PHI_0 at 1 MHz and SCK at 125 kHz with an 8 µs pending-request hold. It does not prove actual post-fit timing, higher SCK, clock-routing or setup/hold margins.

**Proposed Change**

1. **Reproducible toolchain:** Record `atf15xx_yosys`/Yosys/fitter versions, exact target (`ATF1508ASV-15JU84` if that is the device selected for each CPLD), synthesis flags, top module and constraints; run the current committed branch source.
2. **MMU fit:** Incorporate both shared `BUS_RD_N`/`BUS_WR_N` outputs plus all existing control pins. Verify I/O count, macrocells, combinational logic and fitter timing; inspect mutually exclusive strobes and any output/clock pin constraints.
3. **IOU fit:** Incorporate the full 13-bit SPI receiver, five-bit selector decode and saturated counter. Record macrocell, I/O and clock/timing results and compare with the previous revision where feasible.
4. **Report and disposition:** Attach commands, tool output, final fit summaries and any limitations. If fitting or timing fails, document the exact violation and create a focused revised RTL change for review. Do not silently delete features to make the fitter pass.
5. **Release gate:** Until reports pass, classify the project as **Verilator/emulator verified but CPLD fitting unverified**, irrespective of PR merge status.

**Acceptance criteria / required evidence**

- [ ] Intended ATF1508ASV device/package, top modules, build definitions and exact toolchain documented.
- [ ] MMU synthesised and fitted with the new two shared bus strobes; macrocells/pins/timing reported.
- [ ] IOU synthesised and fitted with corrected SPI slave; macrocells/pins/timing reported.
- [ ] Resource/timing margins and any routing or clock limitations recorded; regressions repeated for RTL fixes if needed.
- [ ] No physical board design or bench verification claimed.
- [ ] Owner/reviewer accepts the fit evidence before marking **RESOLVED**.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

**DEFERRED** pending `atf15xx_yosys` installation. This is a necessary pre-hardware **software synthesis/fitting gate**, not a blocker to accepting or integrating the already-reviewed emulator/RTL functionality.
