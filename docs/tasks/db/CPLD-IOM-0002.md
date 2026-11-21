# CPLD-IOM-0002 — Lock 16-bit IOMEM disk command protocol and update references

**ID:** `CPLD-IOM-0002`  
**Status:** `OPEN`  
**Title:** Lock 16-bit IOMEM disk command protocol and update references

**Description**

Replace the *proposal* of a 32-bit READ_LBA request in the reference documents with an approved HDD-specific **16-bit block index** contract, aligned with ProDOS and a 512-byte block, without changing the 13-bit SPI transport or Apple mailbox.

**Details**

- Kind: TASK
- Category: `CPLD-IOM`
- Milestone: M3 — IOMEM disk service
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: None (may start independently)

### Existing evidence, assumptions and related source

- `docs/iomem-command-implementation-reference.md` currently proposes opcode 0x40 and a 32-bit big-endian LBA example, explicitly not implemented or locked.
- `emulator/slot_peripherals/hard_drive.c` already uses a 16-bit `fifo_block_num`; ProDOS `$46–$47` is also a 16-bit block field.
- `docs/spi-iomem-protocol.md` proposes one-byte ACK 0x60 followed by final status + a two-byte payload length; this is a proposed application layer, not existing wire logic.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.

**Proposed Change**

1. Define a versioned opcode registry and exact on-wire request/reply bytes for GET_STATUS/GET_SIZE, READ_BLOCK and WRITE_BLOCK (optionally FORMAT/RESET); allocate opcodes only after checking collisions.
2. Recommended READ request shape: `opcode, unit, block_lo, block_hi` (total 4 bytes), matching ProDOS little-endian block order; lock endianness explicitly during review. GET_SIZE must specify how 65536 blocks or larger/unsupported media are reported.
3. Preserve existing opcode-accept ACK and existing mailbox pending handshake; choose exact final status/length/error and sector-write acknowledgement semantics. Document unit select, read-only mode, `0x0000`/`0xffff` bounds, size cap and multi-drive behaviour.
4. Update both earlier protocol/example documents when the protocol is accepted. Do not leave the 32-bit example silently contradictory: mark it historical or revise it and record the breaking proposal change.
5. Add a byte-exact 512-byte READ walkthrough and WRITE request tests with data 0x00/0xFF, timeout/resync rules and no presumed SPI burst capability.

### Acceptance criteria and evidence to record

- [ ] A reviewed byte-level request/response table is sufficient to implement Apple firmware and STM32 controller independently.
- [ ] Block index is exactly 16 bits throughout HDD path; one 512-byte block is transferred without sentinel interpretation.
- [ ] No IOMEM command requires a new CPLD SPI selector or a changed Apple C0Fx register.
- [ ] The existing 32-bit proposal is clearly superseded in documentation.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
