# CPLD-FW-0001 — Define ProDOS slot 7 ROM signature, entry points and boot layout

**ID:** `CPLD-FW-0001`  
**Status:** `OPEN`  
**Title:** Define ProDOS slot 7 ROM signature, entry points and boot layout

**Description**

Create an Apple 6502/65C816-compatible slot7 firmware skeleton with the ProDOS block-driver identification, entry points, a safe boot path and an expansion-ROM policy before writing disk-transfer routines.

**Details**

- Kind: TASK
- Category: `CPLD-FW`
- Milestone: M2 — Slot 7 firmware
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: [`CPLD-ROM-0001`](CPLD-ROM-0001.md), [`CPLD-ROM-0002`](CPLD-ROM-0002.md)

### Existing evidence, assumptions and related source

- ProDOS block-device signature for slot n: `$Cn01 = $20`, `$Cn03 = $00`, `$Cn05 = $03` (therefore slot7 C701/C703/C705). `$CnFF` is the offset of the block driver entry; `$CnFE` gives capabilities; `$CnFC–$CnFD` may publish block count.
- ProDOS driver parameters: zero-page `$42` command, `$43` unit, `$44–$45` data buffer pointer, `$46–$47` 16-bit block number; STATUS/READ/WRITE/FORMAT; carry/A error results. The ROM must preserve/register calling convention.
- Sources: https://prodos8.com/docs/techref/adding-routines-to-prodos/ and https://prodos8.com/docs/technote/20/ ; avoid assuming a SmartPort implementation is mandatory.
- Boot entry `$C700` and ProDOS recognition/entry can differ; their exact startup workflow must be validated against a real ProDOS image.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.

**Proposed Change**

1. Write `docs/prodos-slot7-firmware.md` with bootstrap entry, ProDOS probe signature, `$C7FC–$C7FF` bytes, supported commands and advertised capability flags. Start with read-only capabilities if WRITE is not yet implemented; never advertise unsupported operations.
2. Select an assembler/toolchain and define reproducible 256-byte slot ROM + optional C800 expansion image layout with deterministic fill/padding, linked to the ROM map.
3. Build a minimal ROM that ProDOS recognises at slot7 and can invoke for STATUS with a clearly controlled placeholder result; no IOMEM sector access is needed yet.
4. Decide compatibility mode explicitly (ProDOS block device first; SmartPort only if separately justified). Specify interrupt, bank register, zero-page and software-switch preservation.

### Acceptance criteria and evidence to record

- [ ] Generated ROM contains correct slot7 signature and metadata; its entry point stays within the selected ROM window.
- [ ] Emulator reads the exact generated ROM bytes through ROM_EN_N/MEM_ADDR_OUT, including entry point bytes.
- [ ] ProDOS device enumeration sees a slot7 block device or diagnostic evidence precisely identifies the remaining discrepancy; no false boot claim.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
