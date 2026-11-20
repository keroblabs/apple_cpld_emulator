# CPLD-HDD-0002 — Attach existing disk image and validate safe read/write backend

**ID:** `CPLD-HDD-0002`  
**Status:** `OPEN`  
**Title:** Attach existing disk image and validate safe read/write backend

**Description**

Connect the user's existing hard-disk image to the new IOMEM disk backend and confirm block geometry, byte-accurate reads, error handling and a safe strategy for writes.

**Details**

- Kind: TASK
- Category: `CPLD-HDD`
- Milestone: M4 — Hard-disk integration
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: [`CPLD-HDD-0001`](CPLD-HDD-0001.md)

### Existing evidence, assumptions and related source

- `emulator/slot_peripherals/hard_drive.c` uses `filesys_drv_image_access()` with `Disc_Mode_Read`, `Disc_Mode_Write` and `Disc_Mode_Size`, 512-byte sectors and selected image paths; the image data itself is not in the inspected git tree.
- Current legacy interface returns 16-bit block counts and offsets via `block_num << 9`; the new backend must validate image size and checked offsets, not assume all files are valid ProDOS volumes.
- The user has the intended image; this ticket does not assume it has already been uploaded or committed.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.

**Proposed Change**

1. Define a configurable image path and test fixture, verify actual image type (raw sectors vs container/header), sector size, block count, bootability and ProDOS compatibility before integration.
2. Add read-only mode first and test block0/block1 and boundary blocks against direct file reads; preserve the original image unchanged.
3. Implement guarded writes against a disposable image copy or copy-on-write overlay only after protocol/ROM advertises write support; test exact round-trip on multiple blocks, invalid blocks and recovery after failure.
4. Define image size handling for 16-bit block addressing (including an exact 65536-block boundary and image sizes not multiple of 512); provide configuration and diagnostics.

### Acceptance criteria and evidence to record

- [ ] READ_BLOCK from IOMEM yields exactly the same 512 bytes as a direct read of the selected backing image at `block_index*512`.
- [ ] The original source image remains unmodified throughout smoke/regression testing.
- [ ] Writes (when enabled) persist only to an explicitly writable disposable image/overlay; read-only and errors behave as advertised by slot7 ROM.
- [ ] Document image format, test fixture provenance, selected unit and 16-bit capacity constraints.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
