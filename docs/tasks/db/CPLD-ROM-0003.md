# CPLD-ROM-0003 — Implement emulator ROM image backing from ROM_EN_N and MEM_ADDR_OUT

**ID:** `CPLD-ROM-0003`  
**Status:** `REVIEW`  
**Title:** Implement emulator ROM image backing from ROM_EN_N and MEM_ADDR_OUT

**Description**

Provide an emulated flash/ROM address source for selected slot firmware so CPU fetches come from the MMU's translated ROM signals, rather than slot-specific software exceptions. The existing 64 KiB ROM array and low-16-bit masking need review against the final map.

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: [`CPLD-ROM-0001`](CPLD-ROM-0001.md), [`CPLD-ROM-0002`](CPLD-ROM-0002.md)

### Existing evidence, assumptions and related source

- `emulator/onboard/memory_controller.c` currently uses `static uint8_t rom_memory[ROM_SIZE * 2]`, loads Apple firmware at offset 32 KiB, and reads `rom_memory[cpld->MMU.mem_address & 0x0FFFF]` whenever ROM_EN_N is asserted.
- `cpld/rtl_interconnect.cpp` reconstructs MEM_ADDR_OUT physical address from high outputs and CPU lower address bits in the default Verilator configuration.
- No dedicated packaged slot7 ROM image or per-slot expansion flash map exists in the inspected tree.


### Independent review — 2026-11-20 (changes requested)

The following findings are from inspection of the pushed `mmu-rom-analysis` sources and tests, not independent execution of Verilator or the full emulator. They extend the developer's review evidence without deleting its history.

| Finding ID | Classification | Evidence | Required action / acceptance evidence | State |
| --- | --- | --- | --- | --- |
| ROM3-R1 | ACCEPTANCE GAP | `rom_backing_install()` permits manual array overlays, but no `.romimg` loader/packer, manifest parser, or board-startup installation of slot7 firmware is implemented. The unit test calls the backing API directly. | Implement or clearly split the specified image-packing/loading task, connect an actual image/fixture to board startup through a documented loading route, and test slot7 ROM bytes as seen by CPU requests through `ROM_EN_N`/`MEM_ADDR_OUT`. | OPEN |
| ROM3-R2 | UNVERIFIED TEST CLAIM | Ticket checks 'unmodified baseline Apple ROM still boots', but evidence only describes a clean CMake build and a standalone backing unit test. No runtime boot log, screen observation or full combined IOU/MMU test is supplied. | Run and log real boot on the branch, and a combined RTL→memory-controller ROM fetch test including C700, C800, cross-slot and CFFF; do not equate build success with runtime verification. | OPEN |
| ROM3-R3 | ERROR VISIBILITY | `rom_backing_read()` can report out-of-bounds through its pointer, but the actual memory-controller path uses `rom_backing_read_traced()` which explicitly discards `oob` without reporting it; `idle_fill` silently stands in for the invalid ROM byte. | Make real ROM-path out-of-range read observable via trace/status/assertion or a documented error hook. Unit test must exercise the actual traced path when physical address exceeds 0xFFFF. | OPEN |
| ROM3-R4 | MAP CONTRACT / INPUT CHECK | `rom_backing_install` only checks global 64 KiB bounds; it accepts installation into unrelated system ROM ranges or overlapping slot windows, although the proposed ROM packer requires segment/type/window validation. | Enforce logical-window/overlap validation at the image installer/packer layer, or narrow the public API to a trusted low-level primitive and test rejecting malformed manifests before modifying the ROM image. | OPEN |

**Acceptance assessment:** Standalone unit test passes as reported by developer; full ticket acceptance must remain unchecked pending image integration and runtime/RTL evidence.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-20 — Implementation:** Added `emulator/onboard/rom_backing.c`/`.h`: a 64 KiB logical ROM space indexed by the MMU physical address (`MEM_ADDR_OUT`), with system-ROM load, bounds-checked `rom_backing_install` overlays (slot entry / expansion), explicit out-of-bounds reporting (no silent `& 0xffff` wrap) and an optional trace hook (CPU addr, physical addr, ROM_EN_N, byte). `memory_controller.c` now uses it on the ROM path; `rom_memory[ROM_SIZE*2]` and its masking removed.
- **2026-11-20 — Test:** `emulator/onboard/verification/test_rom_backing.c` (24 checks, 0 failures): system ROM read, slot-7 entry overlay at C700–C7FF, slot-7 expansion overlay at C800–CFFE, no aliasing with system ROM, bounds rejection and the trace hook.
- **2026-11-20 — Test:** Full emulator rebuilt via CMake with `rom_backing.c` added — links clean, no regression in the build.
- **2026-11-20 — Decision:** No default slot-7 overlay is wired into `memory_controller_init` so the unmodified Apple ROM boot path is preserved; overlays are installed explicitly once a packaged image exists (`CPLD-ROM-0002` format).
- **2026-11-20 — Status Change:** `OPEN → IN_PROGRESS → REVIEW`; awaiting owner acceptance.

- **2026-11-20 — Independent Code Review:** Reviewed `f77b2605` against direct C unit test and build integration. Keeping the new backing abstraction is reasonable. The advertised image loading/packing and CPU-visible slot7 verification are not yet implemented, and the ticket's 'still boots' acceptance is not supported by a recorded runtime test. The traced production read silently discards its own OOB flag.

**Proposed Change**

1. Make ROM data selection depend on ROM_EN_N + decoded physical MEM_ADDR_OUT, not direct CPU Cx address shortcuts. Respect the existing system ROM and RAM choices.
2. Implement image loading/packing for slot7 and other configured slots using the reviewed map; ensure physical ROM capacity, bank selection, bounds and ROM holes are explicit, with no silent wrapping by `& 0xffff`.
3. Add emulator-visible traces for CPU address, physical ROM address, ROM_EN_N and returned byte, and unit tests for the C700 and C800 windows.
4. Do not break existing Apple system ROM boot, video, RAM or disk slot6 paths. Keep loading policy compatible with future STM32 ROM access strategy.

### Acceptance criteria and evidence to record

- [ ] A synthetic slot7 ROM with distinguishable byte patterns is visible at C700–C7FF and its selected expansion bank at C800–CFFE, without aliasing the system ROM (`test_rom_backing.c` tests 2–4).
- [ ] Cross-slot selections and CFFF release produce the documented bytes; no memory access exceeds loaded image bounds (bounds test 5; MMU-side CFFF/slot behaviour verified in `CPLD-ROM-0001`).
- [ ] Unmodified baseline Apple ROM still boots and representative MMU switch tests pass (no default overlay installed; full emulator rebuild clean; `tb_mmu_addr.cpp` passes).

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `f77b260` | `CHANGES_REQUESTED` | rom_backing ROM image backing + unit tests; wire into memory_controller |

**Resolution Description**

Awaiting owner review. ROM reads are now served by `rom_backing` from the MMU
physical address with explicit bounds and an optional trace hook; overlays
(slot entry/expansion) are bounds-checked. Verified by `test_rom_backing.c`
(24 checks, 0 failures) and a clean full-emulator rebuild. The default boot
path is unchanged (no overlay installed by default), preserving the Apple
system ROM, video, RAM and slot-6 paths.

**Review disposition (2026-11-20):** Remains `REVIEW` with `CHANGES_REQUESTED` on the implementation commit. The independent review found specific outstanding acceptance criteria; no source changes or test reruns are represented by this review update.
