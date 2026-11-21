# CPLD-HW-0003 — Review, fix and optimise IOU SPI-slave RTL

**ID:** `CPLD-HW-0003`  
**Status:** `OPEN`  
**Title:** Review, fix and optimise IOU SPI-slave RTL

**Description**

Perform a **focused software review and improvement** of `rtl_IOU/IOU_spi_iface.v`: serialisation/deserialisation, shifter width, bit count, clock/CS edge handling, pending-handshake timing, reset, and any confirmed logic defects. **Fix issues and improve the RTL** where justified by tests and the ATF1508ASV synthesis/fitter. **Do not change** SPI transport framing, selector/register addressing, Apple-facing mailbox, IOMEM command protocol or other unrelated interfaces.

The STM32 will act as SPI master and will **probably bit-bang the unusual 13-bit exchange**. **Compatibility with a hardware STM32 SPI peripheral, CPOL/CPHA preset, 8-bit framing or hardware-peripheral selection is not a requirement of this ticket.** Retain an exact, deterministic *bit-level* contract that either an STM32 GPIO implementation or the emulator's independent master can produce.

This ticket is limited to **software: Verilog RTL, Verilator/co-simulation, regression tests and CPLD fitting**. **Hardware schematic/PCB design and STM32 firmware implementation are out of scope**; nevertheless RTL must fit the intended ATF1508ASV and meet feasible external pin and clock/timing constraints.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M3 — IOMEM/SPI reliability
- Baseline: `main` inspected 2026-11-21; documentation updated on `cpld-hw-0001-0003-tickets`. Implementation branch per owner/developer workflow.
- Dependencies: [CPLD-HW-0001](CPLD-HW-0001.md) (STM32 ownership). Preserve [SPI/IOMEM contract](../../spi-iomem-protocol.md).
- Related (but excluded): [CPLD-IOM-0003](CPLD-IOM-0003.md) (software queue, backpressure, timeouts and fairness), [CPLD-HW-0002](CPLD-HW-0002.md) (shared memory bus strobes).

**Notes**

- **2026-11-21 — Source evidence:** `IOU_spi_iface.v` currently employs a 4-bit bit counter, 11-bit RX shifter and 8-bit TX shifter with a rising-edge `spi_clk` shift. It loads status on a CS-high clock edge, chooses TX data after five bits, and applies mailbox side effects at the end of the 13-bit word. Confirm exact timing with waveform tests; do not assume an off-the-shelf byte-aligned SPI interface.
- **2026-11-21 — Emulator evidence:** `cpld/rtl_spi_slave.cpp` is named for the simulated peripheral but **drives the master-side pins**; it already models a CS-high preload clock, 13 data bits, and trailing clock in a separate state machine. Keep this abstract, testable bus-driving approach.
- **2026-11-21 — Owner scope correction:** STM32 is expected to use **software bit-banging** because of the nonstandard 12–13-bit transfer. Do not spend this ticket attempting to fit that waveform to the STM32 SPI hardware peripheral or redesign the protocol. Focus on **review, optimisation, improvements and fixing demonstrated bugs**.
- **2026-11-21 — Hardware boundary:** This is software development, including proof that the CPLD can implement the RTL. No actual hardware layout, wiring or STM32 firmware is requested.

**Proposed Change**

1. **Freeze the transport.** Preserve **13-bit MSB-first** transfers: five selector bits plus eight payload bits; simultaneous MISO flags/data layout, defined selectors `0` (consume Apple byte), `1` (publish byte), `2` (keyboard), `5` (status-only), and all current Apple `$C0F1/$C0F2/$C0F3` mailbox and PHI_0 pending-bit semantics. Keep the existing documented preload and trailing clock behaviour unless a waveform-proven bug and an owner-approved compatible correction require otherwise. No register/address/opcode/transport change is authorised.
2. **Baseline first.** Capture Verilator waveforms for each of the thirteen bits, MISO validity relative to SCK edges, CS assertion/deassertion, status load, end-of-frame effects, repeated transactions, keyboard traffic and interleaved CPU/PHI_0 operations. Document a **GPIO/bit-banged timing contract** (which edge the master drives or samples, CS high/low requirements, setup/hold and maximum tested SCK) without requiring a particular STM32 hardware SPI mode. This is *documentation for RTL verification*, **not** STM32 firmware work.
3. **Review for defects and improvements.** Inspect counter width/rollover, receive/transmit shifter sizes, combinational feedback, CS handling, status-shift preload, MOSI/MISO shifting, side effects, mid-transfer CS abort, extra clocks, repeated transactions, synchronous/asynchronous reset assumptions and the SPI↔PHI_0 pending-flag crossing. Fix proven defects; improve legibility/resource use without changing public behaviour. Avoid inferred latches, unbounded/unqualified enables, double set/clear pulses or unintended sequential paths.
4. **Test and integrate minimally.** Add focused RTL tests and end-to-end mailbox/keyboard regression tests. If legitimate RTL corrections alter exact internal edge behaviour, update only the relevant simulated master timing in `cpld/rtl_spi_slave.cpp` and record old/new waveforms; do not rewrite `emulator/onboard/spi_controller.c`, the IOMEM queue, 16-bit HDD protocol, register map or command handlers.
5. **Fit to target.** Run Verilator lint plus actual Yosys/`atf15xx_yosys` synthesis/fitter for the intended ATF1508ASV variant (and selected IOU build definitions). Record baseline versus revised macrocells/I/O pins and timing, feasible bit-clock period, and any constraint breach. An optimisation that increases critical path or fails to fit is not automatically accepted.

**Acceptance criteria / evidence**

- [ ] Existing 13-bit framing, selectors, MISO returned status/data and Apple register semantics remain bit-for-bit compatible.
- [ ] Test vectors include reset/first transfer, CS-high preload, all bit positions, trailing clock, repeated/status-only transfers, keyboard interleaving, aborts/extra clocks, and concurrent PHI_0 accesses.
- [ ] Confirmed bugs fixed with failing-before/passing-after regressions; no lost final bit, spurious pending handshake or unintended writes.
- [ ] A simple bit-banged-master waveform contract is documented, **without requiring STM32 peripheral-mode compatibility**.
- [ ] Original and revised RTL pass Verilator integration tests and existing Apple/keyboard/IOMEM regressions; unrelated emulator protocols unchanged.
- [ ] ATF1508ASV synthesis/fitter/timing and resource comparison included. No PCB or STM32 firmware work performed.
- [ ] If no improvement is justified, provide an evidence-based no-code-change conclusion with tests and fitter results; do not force cosmetic RTL churn.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending RTL source review, regression tests, justified fixes/optimisation and CPLD fitting. Hardware implementation and STM32 firmware are outside this task.
