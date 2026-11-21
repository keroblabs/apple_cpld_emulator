# CPLD-HDD-0001 — Implement onboard IOMEM hard-disk controller and command handlers

**ID:** `CPLD-HDD-0001`  
**Status:** `OPEN`  
**Title:** Implement onboard IOMEM hard-disk controller and command handlers

**Description**

Create the microcontroller-side disk service implementation in the emulator's onboard controller layer, registering the approved 16-bit block IOMEM commands and returning correctly framed 512-byte reads.

**Details**

- Kind: TASK
- Category: `CPLD-HDD`
- Milestone: M3 — IOMEM disk service
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: [`CPLD-IOM-0001`](CPLD-IOM-0001.md), [`CPLD-IOM-0002`](CPLD-IOM-0002.md), [`CPLD-IOM-0003`](CPLD-IOM-0003.md)

### Existing evidence, assumptions and related source

- There is no hard-disk IOMEM registration/handler in `emulator/onboard/`; the current `hard_drive.c` is a separate legacy slot I/O latch/FIFO interface.
- User requested `hhd_controller.c/.h` under `./onboard`; repository path is `emulator/onboard/`. **Naming review:** use conventional `hdd_controller.c/.h` unless the user intentionally wants the `hhd_` spelling; do not create both versions.
- The eventual STM32 firmware should implement the same logical commands, keeping C-side code transport and image source abstract.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.

**Proposed Change**

1. Add `emulator/onboard/hdd_controller.c/.h` (or approved `hhd_` spelling) and register command descriptors after `iomem_controller_init()`, with explicit init and service functions if required.
2. Implement read/status and negotiated write commands using 16-bit block index, unit selection, exact 512-byte buffers, status/length envelope and memory-safe callback lifetimes.
3. Use a block storage backend abstraction rather than copying the old slot I/O command latch code; backend may initially be file-image based in emulator, with future STM32/flash implementation replacing only storage calls.
4. Avoid direct Verilator, SPI IDs and CPU C0Fx register handling in HDD handlers; leave transport in IOMEM/SPI layers. Enforce nonblocking integration or bounded safe state changes.

### Acceptance criteria and evidence to record

- [ ] Handler registry reports STATUS and reads any valid selected block using the locked 16-bit protocol and sends exactly 512 payload bytes.
- [ ] Invalid unit, out-of-range block, read error and write-protect return documented statuses without buffer corruption.
- [ ] No dependency on the legacy slot hard_drive.c path; other IOMEM commands and keyboard continue working.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
