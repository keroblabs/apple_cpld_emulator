# CPLD-TEST-0001 — Prove slot 7 ProDOS boot and full IOMEM hard-disk integration

**ID:** `CPLD-TEST-0001`  
**Status:** `OPEN`  
**Title:** Prove slot 7 ProDOS boot and full IOMEM hard-disk integration

**Description**

Exercise the whole path with real ROM fetch and Apple 65816 execution: ROM_EN_N/MEM_ADDR_OUT → slot7 ProDOS firmware → C0Fx mailbox → SPI → IOMEM HDD service → real image → 512-byte reply → ProDOS boot.

**Details**

- Kind: TASK
- Category: `CPLD-TEST`
- Milestone: M4 — Hard-disk integration
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: [`CPLD-ROM-0003`](CPLD-ROM-0003.md), [`CPLD-FW-0002`](CPLD-FW-0002.md), [`CPLD-HDD-0002`](CPLD-HDD-0002.md)

### Existing evidence, assumptions and related source

- The current emulator is reported working after RTL interface rework, but no slot7 flash ROM firmware, IOMEM HDD handlers or connected ProDOS disk boot through IOMEM have been demonstrated.
- Slot6 optional real-controller C6XX_N compatibility and existing keyboard operation are independent regression requirements.
- A passing synthetic 512-byte command is necessary but not sufficient; actual ProDOS boot must be observed and documented.

**Notes**

- **2026-11-21 — Owner directive / DIP32 single-image handoff:** Final integrated ProDOS/IOMEM test must use the exact **single raw DIP32 flash image** selected for programming, not reconstructed `apple_rom_bin` + patches. Keep image hash and emulator-mode evidence separate from the old `PR#3`/`PR#0` baseline verification.

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-21 — Compiled-image verification:** The emulator runs with the compiled Apple and slot arrays and writes its exact 256 KiB image via `-w`. Verify the runtime backing against the emitted file and use that file for later hardware readback comparison.

**Proposed Change**

### Flash image integration

Boot the emulator with the release slot arrays and `-w IMAGEFILE`. Verify
slot-6 and slot-7 bytes against the export, run ProDOS block reads, and
record the SHA-256 for later hardware readback comparison.

**Cross-ticket gate:** [`CPLD-ROM-0004`](CPLD-ROM-0004.md) and [single-image contract](../../rom-flash-image-contract.md).

1. Automate ROM address testcases, SPI/IOMEM command traces and exact block-data integrity, then boot a known compatible ProDOS image on slot7. Identify the actual boot selection and relevant CPU call traces.
2. Test ProDOS STATUS/READ, filesystem directory listing, repeated block reads, invalid block/media status and, only if enabled, safe block writes to a disposable image.
3. Run regression tests for slot6 floppy decode, keyboard, MMU soft switches, ROM, video and CPU timing/performance. Compare reference behaviour on main or an archived baseline.
4. Measure simulated service time, CPU accesses and complete sector latency; record performance separately from functional correctness, and list all hardware-only STM32 validations pending.

### Acceptance criteria and evidence to record

- [ ] The Apple emulator visibly recognises slot7 ProDOS block device, loads blocks through IOMEM and boots the selected image (or logs a reproducible, precisely diagnosed blocker).
- [ ] 512-byte read correctness verified end to end; repeated reads and error paths do not deadlock, overwrite queues or corrupt memory.
- [ ] Slot6 and keyboard regressions pass and results are attached to the ticket, with named test environment and commit SHA.
- [ ] Review approval required before marking RESOLVED or merging.

### Single-image acceptance gate (added 2026-11-21)

- [ ] Observe slot-7 ProDOS boot and an exact IOMEM 512-byte transfer in the emulator run that writes the corresponding programmer image via `-w`.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
