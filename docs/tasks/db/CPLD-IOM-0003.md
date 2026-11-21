# CPLD-IOM-0003 — Provide SPI queue backpressure, timeouts and fair mailbox progress

**ID:** `CPLD-IOM-0003`  
**Status:** `OPEN`  
**Title:** Provide SPI queue backpressure, timeouts and fair mailbox progress

**Description**

Make large IOMEM responses safe over the currently short shared SPI request/response queues, while keyboard transactions may also be pending.

**Details**

- Kind: TASK
- Category: `CPLD-IOM`
- Milestone: M3 — IOMEM disk service
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: None (may start independently)

### Existing evidence, assumptions and related source

- `emulator/onboard/spi_controller.c` uses two buffers of sizeof(spi_data_t)*8 and the current `spi_controller_data_start()` has no enqueue-failure return.
- `framework/buffer.c` ring buffer can overwrite old entries on full; `spi_controller_check_complete()` cycles nonmatching responses.
- `iomem_controller.c` uses polling-loop timeout counters rather than documented elapsed time; 515-byte successful block replies greatly amplify any dropped-byte or starvation problem.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.

**Proposed Change**

1. Introduce a checked SPI enqueue/complete API preserving unique active request/response ownership; prevent overwriting when the ring is full. Keep backward-compatible wrappers if required for keyboard.
2. Define bounded service-time/elapsed-time deadlines and clear failure paths; do not conflate SPI completion, mailbox pending-bit acknowledgement and command completion.
3. Test interleaved keyboard and IOMEM transactions, queue full, reset halfway through a sector, malformed/duplicate/stale responses and repeated command starts.
4. Keep the 13-bit transport and RTL unchanged for this milestone; measure latencies before considering SPI physical optimisation.

### Acceptance criteria and evidence to record

- [ ] No silent dropped transactions under bounded queue pressure; all failures are observable and recoverable.
- [ ] Sequential 512-byte payload and keyboard traffic both make progress; 0x00 and 0xFF are preserved.
- [ ] Stress and timeout tests give reproducible pass/fail evidence, and the baseline emulator still boots.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
