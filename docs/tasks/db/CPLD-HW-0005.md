# CPLD-HW-0005 — Define authorised parallel-NOR update route

**ID:** `CPLD-HW-0005`  
**Status:** `OPEN`  
**Title:** Define authorised parallel-NOR update route

**Description**

The `SST39VF010-70-4I-NHE` has a useful parallel NOR program/erase capability, which the owner wants to retain and exploit. `CPLD-HW-0002` correctly implements the **two shared bus controls** `BUS_RD_N` and `BUS_WR_N`, but today `BUS_WR_N` is granted only to SRAM. `ROM_EN_N` is read-qualified, no dedicated authorised flash-update select exists, and the emulator has no mutable flash command model. Therefore **in-system NOR flash updates are not implemented**, notwithstanding successful Apple ROM reads and the inherent programmability of the chip.

Investigate and propose the **minimum software/RTL authorisation and command path** for updating CPU-visible parallel NOR while preserving flash mapping, ordinary CPU operation, the two-bus-strobe pin budget and the existing Apple-facing ABI. This is **not an instruction to add registers or start implementation without design approval**.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1 — future controlled NOR update capability
- Baseline: reviewed `CPLD-HW-0002` implementation on `cpld-hw-0001-0003-tickets`
- Dependencies: [CPLD-HW-0001](CPLD-HW-0001.md), [CPLD-HW-0002](CPLD-HW-0002.md), [128 KiB ROM image contract](../../rom-flash-image-contract.md)
- Scope: Architecture review, then **separately approved** Verilog/emulator implementation and simulation tests. **No PCB, wiring, new physical pin assignment, programmer hardware, STM32 firmware or disk SPI flash work.**

**Notes**

- **2026-11-21 — Transfer from HW-0002:** The flash-write path was expressly not implemented or claimed as tested in the shared-strobe ticket. Split it so the finished bus-strobe/emulator work can be RESOLVED without losing the owner's in-system flash-update goal.
- **2026-11-21 — Safety and semantics:** Flash erase/program cycles require valid control, address and command ordering. Ordinary Apple writes to a ROM range must never accidentally issue flash commands. The image writer's 131072-byte `-w` output remains the **startup programmer-image export**, not an implicit persistent runtime flash update. Current CPU ROM decode only addresses the lower 64 KiB; future use of the upper half requires explicit design.

**Proposed Change**

1. Determine which existing CPLD/STM32 or CPU-facing control path can request an **authorised flash write** while preserving the existing `BUS_WR_N`/`BUS_RD_N` pair and independent SRAM/flash selects. Document flash CE#, OE#, WE# logical behaviour without assuming any physical circuit is already designed.
2. Specify access to command/data bus cycles and the selected ROM/flash address, including flash A16 policy, ownership arbitration, write protection, reset/abort and update recovery. Identify unavoidable new state/logic or a potential pin budget problem for owner decision.
3. Preserve the existing Apple ROM and card ROM reads, optional external slot 6, CFFF selection release, 128 KiB image layout, SPI transport, keyboard/IOMEM and all software regressions.
4. Propose a **mutable emulator flash model** with NOR program/erase command semantics and test cases for valid/invalid sequences, incomplete transactions, reset and accidental-ROM-write prevention. Do not modify the compiled startup ROM/export contract.
5. Seek owner **design approval** before implementation; if proposed update logic needs CPLD fitting, include it in the later synthesis gate or an explicitly reviewed extension of HW-0004.

**Acceptance criteria / required evidence**

- [ ] Written source-based update-path proposal and reviewed flash command/control truth table.
- [ ] Owner approves addressing, authorisation, protection and how the shared bus lines serve flash writes.
- [ ] If implementation approved: RTL and emulator implement an actual validated programming route without extra per-device bus strobes or unrelated protocol changes.
- [ ] If implementation approved: positive/negative Verilator tests prove program/erase and that ordinary ROM writes cannot do so.
- [ ] CPLD fit/timing remains a separate outstanding pre-hardware gate; no physical board readiness is implied.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

**OPEN** for future owner-approved design/implementation. The completed HW-0002 bus-strobe change does not implement NOR updates.
