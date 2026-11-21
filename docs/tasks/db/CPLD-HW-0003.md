# CPLD-HW-0003 — Review and optimise IOU SPI-slave serialization and clocking

**ID:** `CPLD-HW-0003`  
**Status:** `OPEN`  
**Title:** Review and optimise IOU SPI-slave serialization and clocking

**Description**

Perform a **focused architectural, logic, waveform and CPLD-fit review** of the existing `rtl_IOU/IOU_spi_iface.v` SPI **slave**. Improve internal bit serialization, clock/CS handling, shifter logic, sampling and timing **where evidence shows a benefit**, without modifying the established transport frame, selectors, register map, Apple-facing mailbox or higher-level IOMEM command protocol. The **STM32 remains SPI master**; no CPLD SPI-master block is required.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M3 — IOMEM/SPI reliability (RTL interface work may run independently)
- Baseline: `main`, inspected 2026-11-21; **implementation branch to be agreed by owner** (ticket prepared on `cpld-hw-0001-0003-tickets`)
- Dependencies: [CPLD-HW-0001](CPLD-HW-0001.md) (STM32 ownership decision); existing [SPI/IOMEM protocol](../../spi-iomem-protocol.md) is the compatibility contract
- Related (separate scope): [CPLD-IOM-0003](CPLD-IOM-0003.md) (software queue/backpressure/timeout/fairness), [CPLD-HW-0002](CPLD-HW-0002.md) (memory strobes)

**Notes**

- **2026-11-21 — Source evidence:** `IOU_spi_iface.v` uses a 4-bit count, an 11-bit RX shifter, an 8-bit TX shifter, and positive `spi_clk` edges. The transmit status shifter loads on a **clock rising edge while CS is inactive**; after five received selector bits it loads either `reg_wr_data` or `8'hFF`; byte side effects occur at bit 13 and pending actions depend on the established PHI_0 mailbox. This unusual edge/preload behaviour needs waveform verification, not assumptions based on an ordinary 8-bit SPI peripheral.
- **2026-11-21 — Emulator evidence:** `cpld/rtl_spi_slave.cpp` (despite its filename) is the **simulated master**. It generates a CS-high preload clock, 13 CS-low bits and a trailing clock, sampling MISO in the low phase before the RTL rising-edge shift. Its scheduler cadence is separate from CPU-bus accesses. The real STM32 must reproduce an electrically valid agreed transaction.
- **2026-11-21 — Current hardware constraint:** Optimisation must respect ATF1508ASV synthesis/fitter and I/O pins, including any clock-capability limitations. Do not claim that simplifying source code necessarily reduces macrocells or improves maximum SCK.

**Proposed Change**

1. **Freeze the existing wire-level contract.** Preserve **13-bit MSB-first transfers**: 5-bit selector then 8-bit payload; MISO status bits (keyboard pending, controller→Apple pending, Apple→controller pending) and returned byte layout; selector 0 read/consume, 1 publish byte, 2 keyboard, 5 status poll; other selectors have no defined mailbox side effect. Keep `$C0F1`/`$C0F2`/`$C0F3` CPU registers, PHI_0 pending/ack rules and byte ordering identical. **Do not redesign the transport, register addresses, device addresses, opcodes or software protocol**.
2. **Establish a golden waveform baseline** before modifying RTL: annotate CS active level, idle/SCK polarity, first valid MISO bit, exact sample/shift edges for all 13 bits, status preload, data phase, status-only reads, bit-13 side-effect timing, deselect and back-to-back transfers. Confirm which SPI mode/STM32 peripheral configuration can support the existing framing; **do not silently change CPOL/CPHA or require a new wire format**. Note any real-MCU incompatibility for a separate owner decision.
3. **Review and optimise internal implementation** if beneficial: shift-register widths and enable logic, bit counter and terminal-count decode, CS handling and state preload, MISO drive/hold timing, removal of redundant combinational paths, explicit no-side-effect handling on malformed/aborted frames, deterministic power/reset behaviour. Avoid latches, multiple drivers, unintended extra pulses and glitch-prone combinational control. Assess clock-domain crossing of SPI actions into PHI_0 mailbox logic without changing mailbox semantics.
4. **Preserve abstraction and co-simulation.** Modify `rtl_IOU/IOU_spi_iface.v` and the focused Verilator tests first. Adjust `cpld/rtl_spi_slave.cpp` **only** if necessary to reflect precisely the approved physical edge timing; keep the independent state-machine scheduler, queue API and IOMEM dispatcher contracts unchanged. Do not rewrite `emulator/onboard/spi_controller.c` or the 16-bit HDD commands as part of this ticket.
5. **Measure actual benefit and fitting.** Run lint/Verilator, synthesise and fit with the existing ATF1508ASV flow. Compare original vs candidate register/macrocell usage, critical clock paths, feasible SCK and timing margin. If the original implementation is already optimal under constraints, deliver the tests and documented no-change recommendation; do not force churn.

**Acceptance criteria / required evidence**

- [ ] Original and revised 13-bit MOSI/MISO frames are **bit-exact compatible** for selectors 0, 1, 2 and 5; status/keyboard/Apple mailbox values unchanged.
- [ ] Waveform tests cover initial transaction after reset, CS-high preload, each of 13 bits, edge polarity, successive frames, status polling, aborted/malformed frames, pending/ack contention and keyboard interleaving.
- [ ] No accidental duplicate pending set/clear action, no loss of final response bit, no unintended register write on CS bounce or clocking outside a valid transfer.
- [ ] Tests prove the PHI_0↔SPI handshake behaviour as observed by the CPU and controller; byte-level IOMEM and existing keyboard paths regress cleanly.
- [ ] Fitter/resource/timing baseline and candidate comparison are recorded; improvement is justified or review-only disposition is explicitly supported by evidence.
- [ ] Any requirement for a physical STM32 SPI mode, pre-CS status preload clock, reset signal or additional hardware pin is called out for owner approval **without** silently altering the locked protocol.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending focused review, waveform regressions and any evidence-based RTL refinement. No new SPI transport or registers are authorised.
