# CPLD-HW-0001 — Freeze STM32 SPI ownership and CPU ROM-read architecture

**ID:** `CPLD-HW-0001`  
**Status:** `OPEN`  
**Title:** Freeze STM32 SPI ownership and CPU ROM-read architecture

**Description**

Formalise SPI master, RTC and serial flash ownership for the STM32. Define
how the CPLD selects and addresses the separate SST39VF010-70 parallel
flash for synchronous CPU reads. The 128 KiB device uses A0–A16 and a
regulated nominal 3.3 V rail. The first 64 KiB serves the CPU and the upper
64 KiB is reserved. [Device decision](CPLD-ROM-0005.md)

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1 — ROM mapping
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: None (may start independently)

### Existing evidence, assumptions and related source

- The current emulation drives the Verilog SPI slave from `cpld/rtl_spi_slave.cpp`; that C++ transport mirrors an eventual STM32 SPI-master role, not a new SPI-master RTL block.
- STM32-side serial flash and the directly addressed CPU flash have separate storage and access timing.
- `MEM_ADDR_OUT` supplies the translated upper address. ROM ownership depends on `ROM_EN_N`, the card strobes, CPU read phase and external device arbitration.

**Notes**

- **2026-11-21 — Owner directive / DIP32 single-image handoff:** The hardware has a **DIP32 flash device for directly accessed ROM**, programmed from one raw binary; this must be distinguished from STM32-owned SPI flash (e.g. disk/media or ancillary storage) and RTC. Earlier language describing only STM32 serial flash as the physical ROM source is not a settled hardware design. Identify the flash chip, address/data pins, voltage, capacity, CE/OE/WE and programming/readback requirements.

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-21 — ROM map reconciliation:** With the 5-bit MMU address interface, card IOSEL uses CPU A7–A0 with physical A10–A8 cleared. Verify the physical low-address selection and the ROM/card enable policy against rev 3, including `C6XX_N` and DIP32 chip capacity.
- **2026-11-21 — Target image size:** The expected hardware contents are the exact 256 KiB startup backing exported by the emulator with `-w`. Verify the part, address wiring and programmer readback against that file.

- **2026-11-21 — ROM review handoff:** The emulator's six per-slot patterns and default image have been verified by the owner, and ROM-0001/0002/0003 are accepted as software. Physical DIP32 acceptance remains OPEN: a 256 KiB flash requires 18 address bits. Current MMU signal exposes only 5 upper bits by default or 17 bits total in the FULL configuration, while actual ROM-read addresses presently occupy 0x00000–0x0FFFF and 0x10000–0x3FFFF are zero-filled. Document the intended physical A16/A17 high-line state, source for A0–A11 (including IOSEL clearing A10–A8), select of internal/slot ROM for OE/CE, contention with C6XX_N, and read timing. Test with a populated target before hardware acceptance.

- **2026-11-21 — Device selection supersession:** The prior 256 KiB, 18-bit note is historical. Owner now requires SST39VF010-70 **128 KiB**, A0–A16, with A16 low on current ROM cycles and no banking. Validate 70 ns timing, CE#/OE#/WE#, PLCC/TSOP-to-DIP32 issue, and 3.3 V versus 5 V electrical compatibility. Link review and eventually physical readback to CPLD-ROM-0005; do not claim physical approval yet.
- **2026-11-21 — Reference interface:** ROM-0005 supplies the device pin map, A16 strap, low-address selection, voltage and bus timing requirements below. The exact package, socket netlist, translator, measured timing and chip readback still require hardware evidence.

- **2026-11-21 — Accepted software handoff / explicit hardware ownership:** The reviewer approved the emulator image/export software in CPLD-ROM-0004 and ROM-0005 and moved those tickets to RESOLVED. **All physical hardware obligations remain assigned to this OPEN ticket.** In particular, obtain/identify full SST39VF010-70 order code and PLCC32/TSOP32-to-DIP32 mechanical solution (if required), verify voltage domain/level shifting and power sequencing, flash A16=0 strap, real IOSEL lower-address mux, CE#/OE#/WE# grants and bus high-Z ownership including optional slot6, CPU access/disable timing, and programmer compatibility. Program **exactly the 131072-byte `-w` output** and compare every byte of whole-chip readback and SHA-256; record part ID, adapter/pinout, PCB revision, programmer settings, firmware source commit and binary digest. This is **not completed by the emulator tests**. A populated slot7 ProDOS/IOMEM fixture is separately required by CPLD-TEST-0001.

**Proposed Change**

### Parallel flash integration

Place the parallel flash on the translated MMU address and CPU data read
path. The programmer takes the exact 131072-byte backing exported by the
emulator. It must address both banks and program the reserved bank to zero.

**Cross-ticket gate:** [`CPLD-ROM-0004`](CPLD-ROM-0004.md) and [single-image contract](../../rom-flash-image-contract.md).

1. Document a hardware block diagram and the exact direction/voltage/timing/ownership of STM32↔CPLD SPI control, RTC and flash, CPU memory bus, ROM_EN_N, MEM_ADDR_OUT and DATA bus.
2. Verify that the flash bus, address selection and chip-enable wiring meet CPU-cycle timing and the physical ROM map.
3. Keep optional physical slot6 drive-controller C6XX_N signal and confirm no bus contention; no new CPLD-side SPI master.
4. Use the reference connections below for the board design and record the implemented netlist, electrical checks and timing measurements.

### SST39VF010-70 reference connections

Microchip DS20005023E lists `SST39VF010-70-4C-NHE` in PLCC32 and
`SST39VF010-70-4C-TU` in TSOP32. Confirm the actual suffix, footprint and
pin-1 orientation. For a DIP32 socket, review the adapter's pin-by-pin
socket mapping and voltage domains or obtain approval for a footprint
change. The table gives the **device** pin numbers viewed from the top.
[Microchip datasheet, Figure 2-1 and ordering codes](https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/SST39LF010-SST39LF020-SST39LF040-SST39VF010-SST39VF020-SST39VF040-Data-Sheet-DS20005023.pdf)

| Device signal | PLCC32 pin | TSOP32 pin | Target connection |
| --- | ---: | ---: | --- |
| A0 | 12 | 20 | Translated CPU A0 |
| A1 | 11 | 19 | Translated CPU A1 |
| A2 | 10 | 18 | Translated CPU A2 |
| A3 | 9 | 17 | Translated CPU A3 |
| A4 | 8 | 16 | Translated CPU A4 |
| A5 | 7 | 15 | Translated CPU A5 |
| A6 | 6 | 14 | Translated CPU A6 |
| A7 | 5 | 13 | Translated CPU A7 |
| A8 | 27 | 3 | Low for card IOSEL, translated CPU A8 otherwise |
| A9 | 26 | 2 | Low for card IOSEL, translated CPU A9 otherwise |
| A10 | 23 | 31 | Low for card IOSEL, translated CPU A10 otherwise |
| A11 | 25 | 1 | Low for card IOSEL, translated CPU A11 otherwise |
| A12 | 4 | 12 | MMU translated address bit 12 |
| A13 | 28 | 4 | MMU translated address bit 13 |
| A14 | 29 | 5 | MMU translated address bit 14 |
| A15 | 3 | 11 | MMU translated address bit 15 |
| A16 | 2 | 10 | Strap to VSS. The flash has no driven bank-select signal on the target. |
| DQ0 | 13 | 21 | CPU D0 through the verified data interface |
| DQ1 | 14 | 22 | CPU D1 through the verified data interface |
| DQ2 | 15 | 23 | CPU D2 through the verified data interface |
| DQ3 | 17 | 25 | CPU D3 through the verified data interface |
| DQ4 | 18 | 26 | CPU D4 through the verified data interface |
| DQ5 | 19 | 27 | CPU D5 through the verified data interface |
| DQ6 | 20 | 28 | CPU D6 through the verified data interface |
| DQ7 | 21 | 29 | CPU D7 through the verified data interface |
| CE# | 22 | 30 | Active-low flash read grant, high in reset and idle |
| OE# | 24 | 32 | Active-low output enable for the granted read phase |
| WE# | 31 | 7 | Held high at flash VDD on the target |
| VDD | 32 | 8 | Regulated 3.3 V with local bypassing |
| VSS | 16 | 24 | Ground and signal return |
| NC | 1, 30 | 6, 9 | Leave open |

For the 5-bit MMU interface, reconstruct A11–A0 with the mux above and
take A15–A12 from the four lower pins of `MEM_ADDR_OUT[16:12]`. The
`FULL_MMU_ADDR` build already supplies the muxed lower bits. The RAM
bank line remains separate. For a card `$C6xx` read, select the same low
address as IOSEL. When internal Apple ROM owns `$C6xx`, pass CPU A11–A8
through to reach the original Apple offset.

Qualify every flash grant with power ready, reset released, CPU read, and
the valid bus phase. Use the same ownership to gate any external data
buffer. During reset, unpowered intervals, RAM/I/O, and writes, keep the
flash disabled and its path to the CPU bus high impedance. Bias CE# and
OE# high until the interface can drive valid levels. Keep WE# high through
reset and tie it to the flash supply on the target. Program and read back
the part with A16 and WE# under programmer control off-board.

| Granted CPU cycle | Flash and CPU data ownership |
| --- | --- |
| Internal Apple Cx or system ROM | Flash selected via the internal ROM decode. An asserted `C6XX_N` during internal `$C6xx` leaves the external card's ROM data path isolated. |
| Flash-owned card IOSEL and IOSTB | Flash selected via the owned card decode, including `C6XX_N` for compiled slot 6. With `MODE_SEL=1`, `ROM_EN_N` can remain high on these reads. With `MODE_SEL=0` it also asserts for the ordinary external IOSEL/IOSTB cases. |
| External card owns slot-6 ROM or its expansion | Flash disabled. The external card alone may drive the CPU data bus. Qualify IOSTB with the latched slot owner. |
| RAM, I/O, CPU write, reset or idle | Flash disabled. An I/O device owns its own range, including the floppy soft switches when configured. A `$CFFF` write releases the selector without enabling ROM data. |

Check the latched owner through the completed `$CFFF` read and release
it for the next cycle. A raw OR of the strobes cannot distinguish internal
`$C6xx` from external card access or grant exclusive expansion ownership.

Verify all flash inputs stay within operating limits and translate any
5 V address or control source to 3.3 V. Check the 3.3 V DQ output against
the actual Apple-side VIH and loading and use a tri-state translator or
buffer where required. Verify both rails' startup and power-off isolation
to avoid back-powering either side. The VF operating range in the AC
table is 2.7–3.6 V, while Table 2-1 lists 3.0–3.6 V. The nominal 3.3 V
rail is within both ranges. Confirm limits for the selected order code.
[Microchip datasheet, Tables 2-1, 6-1 and 7-1](https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/SST39LF010-SST39LF020-SST39LF040-SST39VF010-SST39VF020-SST39VF040-Data-Sheet-DS20005023.pdf)

| Read timing item | VF-70 limit | Board check |
| --- | --- | --- |
| Power to first read | 100 µs recommended minimum | Qualify the read grant after rail settling and reset release. |
| Read cycle `tRC` | 70 ns minimum | Check the shortest interval between read addresses at the device. |
| Address and chip enable `tAA`, `tCE` | 70 ns maximum each | Include the address mux, CPLD decode and level shifting before the signals reach the flash. |
| Output enable `tOE` | 35 ns maximum | Use the latest of address valid + 70 ns, CE# low + 70 ns and OE# low + 35 ns. Add data path propagation and leave CPU setup margin before sampling. |
| Output hold `tOH` | 0 ns minimum from address change | Hold address and grant through the board's CPU sampling and hold interval. |
| Output disable `tCHZ`, `tOHZ` | 25 ns maximum | Allow for flash and external-buffer disable delays before the next driver can enable. Output can start driving immediately after enable. |

Measure the CPU/PHI_0 setup and hold margins and the bus turnaround,
especially on internal/card `$C6xx` transitions. The limits are from
[Microchip datasheet, Tables 6-2 and 7-1](https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/SST39LF010-SST39LF020-SST39LF040-SST39VF010-SST39VF020-SST39VF040-Data-Sheet-DS20005023.pdf).

### Acceptance criteria and evidence to record

- [ ] Agreed topology states STM32 SPI master, RTC and SPI flash ownership and how reads of physical ROM addresses return CPU bytes in time.
- [ ] No unstated assumption that the STM32 can stream arbitrary flash reads synchronously at CPU cycle speed.
- [ ] Required address/data/control nets, external slot6 coexistence and reset/boot image ownership are documented.

### Single-image acceptance gate (added 2026-11-21)

- [ ] The purchased SST39VF010-70 suffix, footprint or adapter, voltage interfaces, address mux and bus arbitration are verified on the target.
- [ ] Program the SST39VF010-70 using the unchanged `-w` export and compare the full 131072-byte chip readback byte for byte and by SHA-256. Record device ID, complete part suffix, adapter/PCB version, programmer settings, source revision and exported-image hash. Software export is accepted under [CPLD-ROM-0004](CPLD-ROM-0004.md) and [CPLD-ROM-0005](CPLD-ROM-0005.md); **physical readback is exclusively this ticket's unresolved gate**.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `cf47ae7` | `UNREVIEWED` | Record SST39VF010-70 reference connections, timing and hardware verification gates |

**Resolution Description**

Pending physical verification. The reference interface is ready for schematic review.
