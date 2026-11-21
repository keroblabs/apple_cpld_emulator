# CPLD-HW-0001 — Freeze STM32 ownership and 128 KiB PLCC flash architecture

**ID:** `CPLD-HW-0001`  
**Status:** `RESOLVED`  
**Title:** Freeze STM32 ownership and 128 KiB PLCC flash architecture

**Description**

Record three **owner-approved architecture decisions**. This is a **decision/documentation ticket only**: there is **no RTL, emulator, STM32 firmware, PCB or physical-programming implementation** to perform under HW-0001. It does not claim hardware readiness.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1 — ROM mapping / hardware decisions
- Baseline: `main`, reviewed 2026-11-21; ticket-only documentation branch `cpld-hw-0001-0003-tickets`
- Dependencies: [CPLD-ROM-0005](CPLD-ROM-0005.md) (approved 128 KiB image mapping)
- Follow-on implementation: [CPLD-HW-0002](CPLD-HW-0002.md) (memory read/write strobes); [CPLD-HW-0003](CPLD-HW-0003.md) (SPI-slave optimisation)

**Notes**

- **2026-11-20 — Initial design:** The STM32 owns the SPI master, RTC and serial storage. The CPLD IOU supplies an SPI **slave** for the established mailbox; no CPLD SPI-master block is required.
- **2026-11-21 — ROM capacity decision:** The target changed from a historical 256 KiB proposal to an approved **128 KiB** flash image with 64 KiB allocated and 64 KiB reserved, as implemented and software-reviewed in ROM-0005.
- **2026-11-21 — Physical package correction:** Use **`SST39VF010-70-4I-NHE`**, a **PLCC32** part (not a DIP32 device). The industrial `4I` suffix replaces the earlier commercial `4C` example because the latter is unavailable to the owner. The manufacturer's DS20005023E identifies 4I as industrial (−40 °C to +85 °C), NHE as PLCC32, and the part as 128K × 8, nominal 3.3 V, 70 ns.
- **2026-11-21 — Scope correction:** Earlier HW-0001 text assigned physical electrical tests, pin mapping, chip programming and readback to this ticket. That earlier working reference is [archived unchanged in substance](../archive/CPLD-HW-0001-pre-rescope.md); **these tests have not been performed** and are not HW-0001's closure criteria. Board bring-up and electrical/programmer verification require separately tracked approval before fabrication/physical acceptance.

**Proposed Change — accepted architecture (no code change)**

1. **SPI / storage ownership.** The STM32 is the SPI **master** and directly controls the **serial SPI flash backing the hard drive**, as well as the RTC as appropriate. The CPLD is a **SPI slave** for STM32↔Apple mailbox exchange. Do not add a SPI master to CPLD RTL; do not route hard-disk serial-flash transactions through the CPLD.
2. **Parallel ROM component.** Use **Microchip SST39VF010-70-4I-NHE**, **128 KiB (1 Mbit, 128K × 8), 70 ns, PLCC32, 3.3 V**. It supplies synchronous Apple CPU-visible ROM reads through the MMU/CPLD and is **independent of the STM32's serial hard-disk flash**. The PCB must provide the proper PLCC32 footprint/socket or an explicitly verified adapter; do not specify the non-existent DIP32 version of this selected part.
3. **Physical 128 KiB address contract.** Preserve [ROM-0005](CPLD-ROM-0005.md) and the [flash image contract](../../rom-flash-image-contract.md):

   | Flash offsets | Contents | CPU access |
   | --- | --- | --- |
   | `0x00000–0x07FFF` | Eight 4 KiB slot windows (0 and 3 default zero; 1,2,4,5,6,7 compiled arrays) | IOSEL/IOSTB selection |
   | `0x08000–0x0FFFF` | Existing 32 KiB Apple ROM | Apple internal Cx/system ROM |
   | `0x10000–0x1FFFF` | Zero-filled, reserved 64 KiB | Not CPU-ROM-addressable now |

   All 17 flash address lines A0–A16 exist at the **programmer**, but **flash A16 is held low on the target** under the current CPU ROM mapping. No banking or use of spare space is authorised. The emulator's optional `-w IMAGEFILE` exports the exact raw **131072-byte** startup backing; the upper 64 KiB remains part of the programmed image.

**Acceptance criteria — architecture record**

- [x] STM32 SPI-master and disk serial-flash ownership unambiguous; CPLD SPI-slave only.
- [x] Exact target part/package `SST39VF010-70-4I-NHE` recorded; no 4C or DIP32 assumption.
- [x] Approved lower 64 KiB map and reserved upper 64 KiB documented with A16-low CPU-ROM policy and full 128 KiB export.
- [x] Implementation and physical acceptance explicitly excluded from this ticket and separated from HW-0002/HW-0003 work.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `cf47ae7` | `SUPERSEDED` (scope only) | Earlier physical pin/timing checklist retained in historical reference |

**Resolution Description**

**RESOLVED — owner-approved architecture decisions recorded; no code or hardware implementation due.** This does not mean that the PLCC32 part has been fitted, voltage-shifted, programmed or read back. HW-0002/HW-0003 are separate OPEN tasks. Owner controls documentation integration/CLOSED status.
