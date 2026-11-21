# CPLD-HW-0002 — Add independent SRAM and parallel-flash RD/WR strobes to RTL and emulator

**ID:** `CPLD-HW-0002`  
**Status:** `OPEN`  
**Title:** Add independent SRAM and parallel-flash RD/WR strobes to RTL and emulator

**Description**

The MMU currently exposes `RAM_EN_N` and `ROM_EN_N`, but not **separate physical read and write control pins for each memory device**. Design, implement, simulate and expose **four independent active-low outputs**: `SRAM_RD_N`, `SRAM_WR_N`, `FLASH_RD_N`, `FLASH_WR_N` (final pin names may follow an agreed schematic convention). The emulator must sample **these actual Verilog outputs** and use them to determine external SRAM/ROM accesses, rather than manufacturing the strobes from software's CPU read/write flag.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1 — ROM mapping / MMU hardware interface
- Baseline: `main`, inspected 2026-11-21; **implementation branch to be agreed by owner** (ticket prepared on `cpld-hw-0001-0003-tickets`)
- Dependencies: [CPLD-HW-0001](CPLD-HW-0001.md) (part/ownership decisions), [CPLD-ROM-0005](CPLD-ROM-0005.md) (address/ROM image contract)
- Scope: MMU RTL, combined Verilator top, C++ RTL adapter, emulator memory-controller logic and tests. **Not** an STM32/SPI-master implementation or a rewrite of the MMU map.

**Notes**

- **2026-11-21 — Source evidence:** `rtl_MMU/MMU.v`, `rtl_MMU/MMU_addr.v` and `rtl_IOU_MMU_TB/IOU_MMU_tb.v` expose `RAM_EN_N` / `ROM_EN_N` today. `rtl_MMU/MMU_addr.v` internally qualifies `rom_enabled` with `CPU_RW`, and its card ROM paths also involve `IOSEL_N`, `IOSTB_N` and `C6XX_N`. The slot-ROM case **cannot safely be represented by `ROM_EN_N` alone**.
- **2026-11-21 — Emulator evidence:** `cpld/rtl_interconnect.cpp` captures enable outputs, but `emulator/onboard/memory_controller.c` currently dispatches reads/writes using enable signals and the caller's `is_read`. This can conceal incorrect physical strobes; the testbench must expose the new pins and the emulator must actually consume them.
- **2026-11-21 — Safety boundary:** The chosen `SST39VF010-70-4I-NHE` is a programmable NOR flash, but **normal Apple CPU writes must not accidentally program/erase it**. No in-circuit flash-programming mode is approved by HW-0001. `FLASH_WR_N` needs its own pin/interface and a safe, explicitly defined inactive behaviour until a future programming path is authorised. The 128 KiB ROM image remains preprogrammed off-board.
- **2026-11-21 — Physical boundary:** This ticket designs/validates the *digital strobes*. PLCC32 footprint, level shifting, full-chip programmer readback and board electrical/timing measurements are **not** deemed completed by RTL tests; record a future board-validation gate.

**Proposed Change**

1. **Specify the output truth table first.** Document active-low levels per cycle for normal SRAM read, SRAM write, internal Apple ROM read, IOSEL/IOSTB card ROM read (including optional external slot 6), RAM/ROM non-selected, CPU write to ROM range, I/O/soft-switch access, idle, reset and DMA. Account for `CPU_RW`, `PHI_0`, address validity, any existing CE grants, and turnaround/setup/hold. No RD and WR to one device may be active together, and no SRAM/flash data drivers may contend.
2. **RTL.** Generate `SRAM_RD_N` / `SRAM_WR_N` and `FLASH_RD_N` / `FLASH_WR_N` as independent top-level MMU outputs, not merely names in the emulator. Update `rtl_MMU/MMU.v`, `rtl_MMU/MMU_addr.v` or a small dedicated decoder as appropriate, and propagate to `rtl_IOU_MMU_TB/IOU_MMU_tb.v`. Retain existing enable outputs if needed for compatibility, but define which are CE/selection and which are OE/WE strobes. Do **not** alter the approved ROM addresses or remove real-card slot 6 ownership.
3. **Flash writes safely disabled.** Expose `FLASH_WR_N`, but **hold it deasserted in every ordinary CPU cycle**. Do not drive it from unqualified `~CPU_RW` and do not enable in-system flash programming without separate owner design approval. The physical PCB may tie WE# high or use a pull-up/controlled interface consistent with this explicit policy; reconcile schematic before fabrication. If future flash writes are needed, raise a new ticket with programming protocol, erase protection and arbitration.
4. **Emulator/cosimulation.** Extend the Verilated top and `cpld/rtl_model.h` / `cpld/rtl_interconnect.cpp` to capture all four new outputs, including idle/reset defaults; use them in `emulator/onboard/memory_controller.c` for RAM/ROM accesses and writes. Preserve the existing SPI-master/IOU interconnect, scheduler separation and `FULL_MMU_ADDR` versus 5-bit build behaviour. A CPU write to the ROM address space must not modify the compiled `rom_backing`.
5. **Fit and timing.** Inspect hardware pin allocation and ATF1508ASV fitter/macrocell use; report whether four extra outputs and any decode fit within the actual chosen CPLD device and available package pins. Prove PHI_0-relative OE/WE pulses, no output glitches during address changes, and no overlap across RAM↔ROM/I/O transition. If pin budget prevents all four, report the precise limitation for owner decision rather than silently multiplexing pins.

**Acceptance criteria / required evidence**

- [ ] Truth table and physical signal polarity documented and reviewed before implementation.
- [ ] Dedicated four RTL outputs visible in combined Verilator model (normal and `FULL_MMU_ADDR` builds), with deterministic inactive reset/idle levels.
- [ ] SRAM reads assert only `SRAM_RD_N`; SRAM writes only `SRAM_WR_N`; authorised flash reads assert only `FLASH_RD_N`; ordinary CPU writes never assert `FLASH_WR_N`.
- [ ] Both internal Apple ROM and selected slot ROM reads use the correct flash grant; C6XX/slot6 external ownership, CFFF release and all I/O/non-memory cases do not generate unintended chip strobes.
- [ ] Emulator **uses RTL-generated outputs** for SRAM/ROM read/write decisions; existing Apple IIe startup, ROM/image parity, memory switching, floppy/slot6 and keyboard regressions pass.
- [ ] Waveform-driven tests cover read→write→read, reset mid-cycle, CPU address changes, bus turnaround, DMA semantics and both MMU address variants. No SRAM/flash contention or spurious write pulse.
- [ ] CPLD pin/fitter report and timing limits are attached, along with actual changed files, commits and reproducible test commands. No claim of completed PCB electrical/ROM-chip validation.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending implementation/review. Ticket creation does not authorise unrelated RTL changes or physical-hardware approval.
