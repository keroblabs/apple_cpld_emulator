# CPLD-IOM-0001 — Expose IOMEM command registration and harden dispatcher

**ID:** `CPLD-IOM-0001`  
**Status:** `OPEN`  
**Title:** Expose IOMEM command registration and harden dispatcher

**Description**

Make `iomem_controller.c` operational as a command registry rather than a dormant list; add public registration and proper request/response completion, including safety on missing handlers and exceptional requests.

**Details**

- Kind: TASK
- Category: `CPLD-IOM`
- Milestone: M3 — IOMEM disk service
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: None (may start independently)

### Existing evidence, assumptions and related source

- `emulator/onboard/iomem_controller.c` has private `iomem_cmd_t`, `iomem_cmd_list`, callbacks and a buffer but no API for registering commands; init currently clears the list.
- Current `start_func(opcode)` is interpreted as total request size including opcode; zero-parameter commands (size 1) are not dispatched immediately. Request length is not bounded before writes.
- Existing opcode ACK = 0x60 and unknown opcode = 0x55; `docs/spi-iomem-protocol.md` distinguishes these from the proposed final response envelope.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.

**Proposed Change**

1. Expose `iomem_cmd_t`, callback types and `iomem_register_command()` in `emulator/onboard/iomem_controller.h`; use stable descriptor lifetimes, duplicate/null checks and deterministic registration order.
2. Validate returned request length 1..528; fix opcode-only requests, buffer bounds, callback invocation counts and reset/abort behaviour. Preserve legacy ACK/reject values unless protocol revision approved.
3. Provide a small synthetic deterministic command/echo test that uses the Apple mailbox and existing SPI queue; do not depend on HDD or ProDOS until registry tests pass.
4. Keep C API nonblocking at the dispatcher level; long device reads must not freeze event processing without an explicit readiness state.

### Acceptance criteria and evidence to record

- [ ] Registered callback executes once per complete command; opcode-only and maximum-length input work; duplicate or invalid registration fails safely.
- [ ] Unknown opcode returns documented rejection; timeout, cancellation and reset return to a recoverable idle state.
- [ ] Existing keyboard via selector2 remains unaffected; no Verilog changes required.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
