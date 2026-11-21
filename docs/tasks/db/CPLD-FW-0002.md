# CPLD-FW-0002 — Implement slot 7 ProDOS block firmware using IOMEM mailbox

**ID:** `CPLD-FW-0002`  
**Status:** `OPEN`  
**Title:** Implement slot 7 ProDOS block firmware using IOMEM mailbox

**Description**

Implement Apple-side STATUS, READ, and later WRITE through the IOMEM mailbox at C0F1/C0F2/C0F3, translating ProDOS zero-page requests into the locked 16-bit IOMEM block protocol. Ensure a ProDOS loader/firmware can obtain an actual sector 0 and start a boot sequence.

**Details**

- Kind: TASK
- Category: `CPLD-FW`
- Milestone: M2 — Slot 7 firmware
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: [`CPLD-FW-0001`](CPLD-FW-0001.md), [`CPLD-ROM-0003`](CPLD-ROM-0003.md), [`CPLD-IOM-0002`](CPLD-IOM-0002.md), [`CPLD-IOM-0001`](CPLD-IOM-0001.md), [`CPLD-IOM-0003`](CPLD-IOM-0003.md)

### Existing evidence, assumptions and related source

- `rtl_IOU/IOU_iomem.v` provides one-byte pending-bit mailboxes at C0F1/C0F2/C0F3; the earlier protocol document defines the transport but its disk example currently describes a **proposed 32-bit** LBA and must be superseded for this HDD milestone.
- ProDOS block number is naturally 16 bits at `$46/$47`; buffer pointer is at `$44/$45`, unit at `$43`, command at `$42`.
- The generated firmware must fit slot ROM/expansion ROM layout and use ROM_EN_N/MEM_ADDR_OUT, not hardcode a special emulator-only C700 path.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.

**Proposed Change**

1. Add a reproducible assembly source, build script and generated ROM image for slot7; firmware entry/boot path and driver callable from ProDOS as documented in FW-0001.
2. Implement mailbox send/receive routines with bounded polling and byte acknowledgements, command ACK checking, payload length/status parsing, and error-to-ProDOS carry/A mapping.
3. Implement STATUS (device present and 16-bit block count), READ (512-byte payload into ProDOS buffer) and explicit WRITE capability/error handling. Add full WRITE only when HDD-0001/0002 backend supports safe writes; FORMAT may report unsupported.
4. Ensure data stream can contain 0x00/0xFF, buffer pointer crossing page boundaries works, zero-page/CPU registers are preserved as ProDOS expects, and returning from an error does not strand pending bytes.
5. Cover card bootstrap behaviour separately: confirm ProDOS can load from slot7 under the chosen ROM mapping and device priority.

### Acceptance criteria and evidence to record

- [ ] A ProDOS diagnostic invokes slot7 STATUS and reads a deterministic 512-byte block through actual C0Fx accesses, validating entire buffer.
- [ ] All firmware-visible timeouts and backend errors are mapped to defined ProDOS status and carry behaviour; no indefinite busy-wait.
- [ ] ROM image fits in documented flash layout and does not break slot6 physical-drive compatibility.
- [ ] ProDOS block driver and Apple firmware do not depend on C++ SPI internals.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
