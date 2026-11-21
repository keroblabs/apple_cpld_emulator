# Apple CPLD Emulator — Task List

**Status:** Initial hard-disk programme tickets opened 2026-11-20 on `rtl-rework`.  
**Methodology:** [Task and Bug Tracking Methodology](bug-tracking-methodology.md).  
**Scope:** Slot7 ProDOS firmware → IOMEM 16-bit block protocol → onboard disk-image service. The STM32 remains the sole SPI master and owns the RTC and serial flash. A separate SST39VF010-70 serves CPU ROM from one raw flash binary. [Flash image contract](../rom-flash-image-contract.md)

The task rows below give the current status of each ticket; design and implementation approvals are tracked in each individual ticket. See each ticket's detailed dependencies and acceptance criteria. Core workstreams: **M1** ROM mapping and hardware ROM-read strategy → **M2** slot7 ProDOS firmware; **M3** IOMEM protocol and hard-disk service can proceed alongside M1/M2 → **M4** image integration and ProDOS boot verification.

The **128 KiB SST39VF010-70** image uses the approved lower 64 KiB ROM map
and reserves the zero-filled upper 64 KiB. The package, 3.3 V interface,
timing and physical readback require hardware validation. [CPLD-ROM-0005](db/CPLD-ROM-0005.md)

**2026-11-21 review update:** CPLD-ROM-0001/0002/0003 accepted for emulator integration (`RESOLVED`) after owner `PR#3`/`PR#0` and slot-pattern verification. CPLD-ROM-0004 software export is approved but remains `REVIEW` until device programming/readback and populated slot7/IOMEM acceptance; CPLD-HW-0001 remains `OPEN` for the electrical design. Integration/merging is owner-controlled.

## CPLD-ROM — ROM mapping and flash layout

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-ROM-0001` | TASK | RESOLVED | Verify MMU slot ROM address generation and control signals | M1 — ROM mapping | [Details](db/CPLD-ROM-0001.md) |
| `CPLD-ROM-0002` | TASK | RESOLVED | Specify slot ROM and expansion ROM flash layout | M1 — ROM mapping | [Details](db/CPLD-ROM-0002.md) |
| `CPLD-ROM-0003` | TASK | RESOLVED | Implement emulator ROM image backing from ROM_EN_N and MEM_ADDR_OUT | M1 — ROM mapping | [Details](db/CPLD-ROM-0003.md) |
| `CPLD-ROM-0004` | TASK | REVIEW | Produce and emulate one programmer-ready flash image | M1 — ROM mapping | [Details](db/CPLD-ROM-0004.md) |
| `CPLD-ROM-0005` | TASK | REVIEW | Retarget ROM image/export and hardware contract to 128 KiB SST39VF010-70 | M1 — ROM mapping | [Details](db/CPLD-ROM-0005.md) |

## CPLD-HW — STM32 / CPLD hardware topology

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-HW-0001` | TASK | OPEN | Freeze STM32 SPI ownership and CPU ROM-read architecture | M1 — ROM mapping | [Details](db/CPLD-HW-0001.md) |

## CPLD-FW — Apple slot 7 ProDOS firmware

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-FW-0001` | TASK | OPEN | Define ProDOS slot 7 ROM signature, entry points and boot layout | M2 — Slot 7 firmware | [Details](db/CPLD-FW-0001.md) |
| `CPLD-FW-0002` | TASK | OPEN | Implement slot 7 ProDOS block firmware using IOMEM mailbox | M2 — Slot 7 firmware | [Details](db/CPLD-FW-0002.md) |

## CPLD-IOM — IOMEM and SPI transport

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-IOM-0001` | TASK | OPEN | Expose IOMEM command registration and harden dispatcher | M3 — IOMEM disk service | [Details](db/CPLD-IOM-0001.md) |
| `CPLD-IOM-0002` | TASK | OPEN | Lock 16-bit IOMEM disk command protocol and update references | M3 — IOMEM disk service | [Details](db/CPLD-IOM-0002.md) |
| `CPLD-IOM-0003` | TASK | OPEN | Provide SPI queue backpressure, timeouts and fair mailbox progress | M3 — IOMEM disk service | [Details](db/CPLD-IOM-0003.md) |

## CPLD-HDD — Hard-disk service and image

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-HDD-0001` | TASK | OPEN | Implement onboard IOMEM hard-disk controller and command handlers | M3 — IOMEM disk service | [Details](db/CPLD-HDD-0001.md) |
| `CPLD-HDD-0002` | TASK | OPEN | Attach existing disk image and validate safe read/write backend | M4 — Hard-disk integration | [Details](db/CPLD-HDD-0002.md) |

## CPLD-TEST — Integration and regression tests

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-TEST-0001` | TASK | OPEN | Prove slot 7 ProDOS boot and full IOMEM hard-disk integration | M4 — Hard-disk integration | [Details](db/CPLD-TEST-0001.md) |

