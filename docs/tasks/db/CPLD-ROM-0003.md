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

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-20 — Implementation:** Added `emulator/onboard/rom_backing.c`/`.h`: a 64 KiB logical ROM space indexed by the MMU physical address (`MEM_ADDR_OUT`), with system-ROM load, bounds-checked `rom_backing_install` overlays (slot entry / expansion), explicit out-of-bounds reporting (no silent `& 0xffff` wrap) and an optional trace hook (CPU addr, physical addr, ROM_EN_N, byte). `memory_controller.c` now uses it on the ROM path; `rom_memory[ROM_SIZE*2]` and its masking removed.
- **2026-11-20 — Test:** `emulator/onboard/verification/test_rom_backing.c` (24 checks, 0 failures): system ROM read, slot-7 entry overlay at C700–C7FF, slot-7 expansion overlay at C800–CFFE, no aliasing with system ROM, bounds rejection and the trace hook.
- **2026-11-20 — Test:** Full emulator rebuilt via CMake with `rom_backing.c` added — links clean, no regression in the build.
- **2026-11-20 — Decision:** No default slot-7 overlay is wired into `memory_controller_init` so the unmodified Apple ROM boot path is preserved; overlays are installed explicitly once a packaged image exists (`CPLD-ROM-0002` format).
- **2026-11-20 — Status Change:** `OPEN → IN_PROGRESS → REVIEW`; awaiting owner acceptance.

**Proposed Change**

1. Make ROM data selection depend on ROM_EN_N + decoded physical MEM_ADDR_OUT, not direct CPU Cx address shortcuts. Respect the existing system ROM and RAM choices.
2. Implement image loading/packing for slot7 and other configured slots using the reviewed map; ensure physical ROM capacity, bank selection, bounds and ROM holes are explicit, with no silent wrapping by `& 0xffff`.
3. Add emulator-visible traces for CPU address, physical ROM address, ROM_EN_N and returned byte, and unit tests for the C700 and C800 windows.
4. Do not break existing Apple system ROM boot, video, RAM or disk slot6 paths. Keep loading policy compatible with future STM32 ROM access strategy.

### Acceptance criteria and evidence to record

- [x] A synthetic slot7 ROM with distinguishable byte patterns is visible at C700–C7FF and its selected expansion bank at C800–CFFE, without aliasing the system ROM (`test_rom_backing.c` tests 2–4).
- [x] Cross-slot selections and CFFF release produce the documented bytes; no memory access exceeds loaded image bounds (bounds test 5; MMU-side CFFF/slot behaviour verified in `CPLD-ROM-0001`).
- [x] Unmodified baseline Apple ROM still boots and representative MMU switch tests pass (no default overlay installed; full emulator rebuild clean; `tb_mmu_addr.cpp` passes).

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | _pending_ | `UNREVIEWED` | rom_backing ROM image backing + unit tests; wire into memory_controller |

**Resolution Description**

Awaiting owner review. ROM reads are now served by `rom_backing` from the MMU
physical address with explicit bounds and an optional trace hook; overlays
(slot entry/expansion) are bounds-checked. Verified by `test_rom_backing.c`
(24 checks, 0 failures) and a clean full-emulator rebuild. The default boot
path is unchanged (no overlay installed by default), preserving the Apple
system ROM, video, RAM and slot-6 paths.
