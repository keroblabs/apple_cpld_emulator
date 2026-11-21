# CPLD-HW-0001 — Freeze STM32 SPI ownership and CPU ROM-read architecture

**ID:** `CPLD-HW-0001`  
**Status:** `OPEN`  
**Title:** Freeze STM32 SPI ownership and CPU ROM-read architecture

**Description**

**2026-11-21 OWNER DEVICE CHANGE:** [CPLD-ROM-0005](CPLD-ROM-0005.md) selects SST39VF010-70, **128 KiB, 17 address bits, nominal 3.3 V**, not a 256 KiB/18-bit part. Verify complete package suffix: vendor documents PLCC32/TSOP32, *not* through-hole DIP32. Physical socket adapter and 5 V/3.3 V interfacing are mandatory topics before PCB approval. Upper 64 KiB is currently reserved; no ROM A16=1 accesses.

Formalise SPI master, RTC and serial flash ownership for the STM32. Define
how the CPLD selects and addresses the separate DIP32 ROM for synchronous
CPU reads.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1 — ROM mapping
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: None (may start independently)

### Existing evidence, assumptions and related source

- The current emulation drives the Verilog SPI slave from `cpld/rtl_spi_slave.cpp`; that C++ transport mirrors an eventual STM32 SPI-master role, not a new SPI-master RTL block.
- STM32-side serial flash and the directly accessed DIP32 CPU ROM have separate storage and access timing.
- `ROM_EN_N` and `MEM_ADDR_OUT` can identify a requested ROM byte; data return timing and any buffer/SRAM/cache/parallel bus interface must be explicit.

**Notes**

- **2026-11-21 — Owner directive / DIP32 single-image handoff:** The hardware has a **DIP32 flash device for directly accessed ROM**, programmed from one raw binary; this must be distinguished from STM32-owned SPI flash (e.g. disk/media or ancillary storage) and RTC. Earlier language describing only STM32 serial flash as the physical ROM source is not a settled hardware design. Identify the flash chip, address/data pins, voltage, capacity, CE/OE/WE and programming/readback requirements.

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-21 — ROM map reconciliation:** With the 5-bit MMU address interface, card IOSEL uses CPU A7–A0 with physical A10–A8 cleared. Verify the physical low-address selection and the ROM/card enable policy against rev 3, including `C6XX_N` and DIP32 chip capacity.
- **2026-11-21 — Target image size:** The expected hardware contents are the exact 256 KiB startup backing exported by the emulator with `-w`. Verify the part, address wiring and programmer readback against that file.

- **2026-11-21 — ROM review handoff:** The emulator's six per-slot patterns and default image have been verified by the owner, and ROM-0001/0002/0003 are accepted as software. Physical DIP32 acceptance remains OPEN: a 256 KiB flash requires 18 address bits. Current MMU signal exposes only 5 upper bits by default or 17 bits total in the FULL configuration, while actual ROM-read addresses presently occupy 0x00000–0x0FFFF and 0x10000–0x3FFFF are zero-filled. Document the intended physical A16/A17 high-line state, source for A0–A11 (including IOSEL clearing A10–A8), select of internal/slot ROM for OE/CE, contention with C6XX_N, and read timing. Test with a populated target before hardware acceptance.

- **2026-11-21 — Device selection supersession:** The prior 256 KiB, 18-bit note is historical. Owner now requires SST39VF010-70 **128 KiB**, A0–A16, with A16 low on current ROM cycles and no banking. Validate 70 ns timing, CE#/OE#/WE#, PLCC/TSOP-to-DIP32 issue, and 3.3 V versus 5 V electrical compatibility. Link review and eventually physical readback to CPLD-ROM-0005; do not claim physical approval yet.

**Proposed Change**

### Mandatory DIP32 flash-image integration

Place the DIP32 ROM on the `MEM_ADDR_OUT` / `ROM_EN_N` / CPU data read
path and document any intervening logic. Verify timing, address-line
wiring and contention behavior. The programmer takes the exact 256 KiB
backing exported by the emulator.

**Cross-ticket gate:** [`CPLD-ROM-0004`](CPLD-ROM-0004.md) and [single-image contract](../../rom-flash-image-contract.md).

1. Document a hardware block diagram and the exact direction/voltage/timing/ownership of STM32↔CPLD SPI control, RTC and flash, CPU memory bus, ROM_EN_N, MEM_ADDR_OUT and DATA bus.
2. Verify that the DIP32 bus, address selection and chip-enable wiring meet the CPU-cycle timing and documented physical ROM map.
3. Keep optional physical slot6 drive-controller C6XX_N signal and confirm no bus contention; no new CPLD-side SPI master.
4. Define what can be simulated now versus deferred to real STM32 firmware and hardware validation; reserve pin connections and level shifting for later.

### Acceptance criteria and evidence to record

- [ ] Agreed topology states STM32 SPI master, RTC and SPI flash ownership and how reads of physical ROM addresses return CPU bytes in time.
- [ ] No unstated assumption that the STM32 can stream arbitrary flash reads synchronously at CPU cycle speed.
- [ ] Required address/data/control nets, external slot6 coexistence and reset/boot image ownership are documented.

### Single-image acceptance gate (added 2026-11-21)

- [ ] A concrete DIP32 part, electrical address/enable mapping and full 256 KiB programming/readback procedure are verified and linked to ROM-0004.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
