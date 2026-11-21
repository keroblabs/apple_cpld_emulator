# Apple CPLD Emulator — Task List

**Status:** Initial hard-disk programme tickets opened 2026-11-20 on `rtl-rework`.  
**Methodology:** [Task and Bug Tracking Methodology](bug-tracking-methodology.md).  
**Scope:** Slot7 ProDOS firmware → IOMEM 16-bit block protocol → onboard disk-image service. The STM32 remains the sole SPI master and owns the RTC and hard-disk serial SPI flash. A separate **PLCC32 SST39VF010-70-4I-NHE** parallel flash serves CPU ROM from one raw 128 KiB image. [Flash image contract](../rom-flash-image-contract.md)

The task rows below give the current status of each ticket; design and implementation approvals are tracked in each individual ticket. See each ticket's detailed dependencies and acceptance criteria. Core workstreams: **M1** ROM mapping and hardware ROM-read strategy → **M2** slot7 ProDOS firmware; **M3** IOMEM protocol and hard-disk service can proceed alongside M1/M2 → **M4** image integration and ProDOS boot verification.

The **128 KiB SST39VF010-70-4I-NHE** image uses the approved lower 64 KiB ROM map
and reserves the zero-filled upper 64 KiB. The package, 3.3 V interface,
timing and physical readback require hardware validation. [CPLD-ROM-0005](db/CPLD-ROM-0005.md)

**2026-11-21 review update:** CPLD-ROM-0001 through CPLD-ROM-0005 remain **RESOLVED** for software and ROM image/export work. CPLD-HW-0001 is RESOLVED as the STM32/parallel-flash architecture decision record. **CPLD-HW-0002 and CPLD-HW-0003 are RESOLVED for independently reviewed RTL and emulator software** after the developer's regressions and owner manual PR#3/PR#0 operation, including 787 MMU checks, 30,010 ROM-bus checks per MMU variant, 59 SPI checks, and HW-0003's 24,000-check SPI↔PHI_0 phase sweep. As of 2026-11-21 the owner reports successful current **MMU and IOU fits on ATF1508ASV PLCC84** through `devel-rtl`, with JTAG ON and Secure OFF. **CPLD-HW-0004** is now **IN_PROGRESS**: detailed timing, actual pin/clock constraints, intended defines, and physical validation remain unproven. **CPLD-HW-0005** is **OPEN** to design and verify the currently missing authorised NOR flash-update route (the new shared write strobe currently writes SRAM only). Neither PCB design nor physical electrical verification is part of these software tickets. CPLD-FW-0001 / CPLD-TEST-0001 remain OPEN. RESOLVED is not CLOSED; owner-controlled PR merge is required for closure.

## CPLD-ROM — ROM mapping and flash layout

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-ROM-0001` | TASK | RESOLVED | Verify MMU slot ROM address generation and control signals | M1 — ROM mapping | [Details](db/CPLD-ROM-0001.md) |
| `CPLD-ROM-0002` | TASK | RESOLVED | Specify slot ROM and expansion ROM flash layout | M1 — ROM mapping | [Details](db/CPLD-ROM-0002.md) |
| `CPLD-ROM-0003` | TASK | RESOLVED | Implement emulator ROM image backing from ROM_EN_N and MEM_ADDR_OUT | M1 — ROM mapping | [Details](db/CPLD-ROM-0003.md) |
| `CPLD-ROM-0004` | TASK | RESOLVED | Generate and verify one flash image in the emulator | M1 — ROM mapping | [Details](db/CPLD-ROM-0004.md) |
| `CPLD-ROM-0005` | TASK | RESOLVED | Retarget ROM image/export and hardware contract to 128 KiB SST39VF010-70 | M1 — ROM mapping | [Details](db/CPLD-ROM-0005.md) |

## CPLD-HW — CPLD architecture and hardware-targeted RTL

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-HW-0001` | TASK | RESOLVED | Freeze STM32 ownership and 128 KiB PLCC flash architecture | M1 — ROM mapping | [Details](db/CPLD-HW-0001.md) |
| `CPLD-HW-0002` | TASK | RESOLVED | Add shared BUS_RD_N/BUS_WR_N controls to RTL and emulator | M1 — ROM mapping | [Details](db/CPLD-HW-0002.md) |
| `CPLD-HW-0003` | TASK | RESOLVED | Review, fix and optimise IOU SPI-slave RTL | M3 — IOMEM disk service | [Details](db/CPLD-HW-0003.md) |
| `CPLD-HW-0004` | TASK | IN_PROGRESS | Move peripheral outputs to MMU; validate CPLD fit and timing | M1/M3 — RTL verification | [Details](db/CPLD-HW-0004.md) |
| `CPLD-HW-0005` | TASK | OPEN | Define authorised parallel-NOR update route | M1 — ROM update capability | [Details](db/CPLD-HW-0005.md) |

## CPLD-BLD — Build automation

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-BLD-0001` | TASK | REVIEW | Remote ATF1508ASV build driver for MMU and IOU | M1/M3 — RTL build infrastructure | [Details](db/CPLD-BLD-0001.md) |

## CPLD-DOC — Documentation and engineering methodology

| ID | Kind | Status | Title | Milestone | Ticket |
| --- | --- | --- | --- | --- | --- |
| `CPLD-DOC-0001` | IMPROVEMENT | REVIEW | Add stakeholder attribution and current-work summary to tickets | Engineering workflow | [Details](db/CPLD-DOC-0001.md) |

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

