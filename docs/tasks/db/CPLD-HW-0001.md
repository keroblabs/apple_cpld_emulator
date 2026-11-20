# CPLD-HW-0001 — Freeze STM32 SPI ownership and CPU ROM-read architecture

**ID:** `CPLD-HW-0001`  
**Status:** `OPEN`  
**Title:** Freeze STM32 SPI ownership and CPU ROM-read architecture

**Description**

Formalise the physical division of labour: STM32 acts as SPI master for the CPLD IOU, and hosts RTC and serial flash; CPLDs retain MMU/IOU ROM-select and address generation. Define how synchronous CPU ROM reads are actually serviced when ROM images reside in serial flash.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1 — ROM mapping
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: None (may start independently)

### Existing evidence, assumptions and related source

- The current emulation drives the Verilog SPI slave from `cpld/rtl_spi_slave.cpp`; that C++ transport mirrors an eventual STM32 SPI-master role, not a new SPI-master RTL block.
- STM32-side SPI flash stores the firmware/image, but serial flash is not automatically a byte-wide, zero-wait-state device on the Apple CPU data bus.
- `ROM_EN_N` and `MEM_ADDR_OUT` can identify a requested ROM byte; data return timing and any buffer/SRAM/cache/parallel bus interface must be explicit.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.

**Proposed Change**

1. Document a hardware block diagram and the exact direction/voltage/timing/ownership of STM32↔CPLD SPI control, RTC and flash, CPU memory bus, ROM_EN_N, MEM_ADDR_OUT and DATA bus.
2. Compare feasible ROM serving approaches (e.g. direct parallel ROM/SRAM mapped to CPLD address outputs, SRAM shadow preloaded from serial flash, or STM32 parallel read service with demonstrably acceptable latency). Select one by measured/derived CPU-cycle requirements and hardware pins.
3. Keep optional physical slot6 drive-controller C6XX_N signal and confirm no bus contention; no new CPLD-side SPI master.
4. Define what can be simulated now versus deferred to real STM32 firmware and hardware validation; reserve pin connections and level shifting for later.

### Acceptance criteria and evidence to record

- [ ] Agreed topology states STM32 SPI master, RTC and SPI flash ownership and how reads of physical ROM addresses return CPU bytes in time.
- [ ] No unstated assumption that the STM32 can stream arbitrary flash reads synchronously at CPU cycle speed.
- [ ] Required address/data/control nets, external slot6 coexistence and reset/boot image ownership are documented.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
