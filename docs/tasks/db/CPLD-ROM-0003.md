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
| ROM3-R1 | ACCEPTANCE GAP | `rom_backing_install()` permits manual array overlays, but no `.romimg` loader/packer, manifest parser, or board-startup installation of slot7 firmware is implemented. The unit test calls the backing API directly. | Implement or clearly split the specified image-packing/loading task, connect an actual image/fixture to board startup through a documented loading route, and test slot7 ROM bytes as seen by CPU requests through `ROM_EN_N`/`MEM_ADDR_OUT`. | RESOLVED — `rom_backing_load_image()` parses the `*.romimg` format with validation; `memory_controller_load_rom_image()` is the documented board-startup route. `test_rom_backing.c` test 7 loads a built image; test 8 reads the exact physical addresses the MMU emits for $C700/$C7FF/$C800 (RTL-verified mapping). |
| ROM3-R2 | UNVERIFIED TEST CLAIM | Ticket checks 'unmodified baseline Apple ROM still boots', but evidence only describes a clean CMake build and a standalone backing unit test. No runtime boot log, screen observation or full combined IOU/MMU test is supplied. | Run and log real boot on the branch, and a combined RTL→memory-controller ROM fetch test including C700, C800, cross-slot and CFFF; do not equate build success with runtime verification. | VERIFIED (OWNER, 2026-11-21) — emulator runs; Applesoft `PR#3` then `PR#0` works; developer's tests cover representative mapping. Slot-7 ProDOS boot is a separate future ticket. |
| ROM3-R3 | ERROR VISIBILITY | `rom_backing_read()` can report out-of-bounds through its pointer, but the actual memory-controller path uses `rom_backing_read_traced()` which explicitly discards `oob` without reporting it; `idle_fill` silently stands in for the invalid ROM byte. | Make real ROM-path out-of-range read observable via trace/status/assertion or a documented error hook. Unit test must exercise the actual traced path when physical address exceeds 0xFFFF. | RESOLVED — the trace callback now carries an `out_of_bounds` flag, `rom_backing_read_traced()` increments `rom_backing_oob_count()`, and `test_rom_backing.c` test 6 exercises the actual traced path with phys > 0xFFFF (flag set, idle returned, count incremented). |
| ROM3-R4 | MAP CONTRACT / INPUT CHECK | `rom_backing_install` only checks global 64 KiB bounds; it accepts installation into unrelated system ROM ranges or overlapping slot windows, although the proposed ROM packer requires segment/type/window validation. | Enforce logical-window/overlap validation at the image installer/packer layer, or narrow the public API to a trusted low-level primitive and test rejecting malformed manifests before modifying the ROM image. | RESOLVED — `rom_backing_install_region(kind, slot, ...)` enforces per-slot 4 KiB geometry (entry ≤256 B in first 2 KiB, expansion ≤2 KiB in second 2 KiB, slot 1–7), and a 256-byte-page occupancy map rejects overlaps (incl. system region). `test_rom_backing.c` test 4 rejects oversize, bad slot, overlap and system-range installs; the loader rejects malformed images. |

**Acceptance assessment:** Standalone unit test passes as reported by developer; full ticket acceptance must remain unchecked pending image integration and runtime/RTL evidence.

**Rework assessment (2026-11-21):** `rom_backing` reworked for the 4 KiB-per-slot
layout with window/overlap validation (`rom_backing_install_region`), a
`*.romimg` loader (`rom_backing_load_image`), a documented board-startup route
(`memory_controller_load_rom_image`), OOB visibility on the traced path, and
baseline migration preserving Apple-visible bytes. Unit test expanded to 45
checks. ROM3-R1/R3/R4 resolved; ROM3-R2 partially addressed (see table).

### Independent re-review — 2026-11-21 (4 KiB/slot image backing)

**Confirmed by source review:** `memory_controller_init()` copies the existing Apple II image's `$Cn00–$CnFF` bytes into each slot's entry at `(n<<12)|(n<<8)`, including original `$C600–$C6FF` into `0x6600–0x66FF`, and replicates the `$C800–$CFFF` baseline at each slot's `+0x800`. It reads ROM only from `ROM_EN_N` and `MEM_ADDR_OUT`. The owner manually verified emulator operation and Applesoft `PR#3` / `PR#0`: **runtime verification done** (owner-reported, not independently reproduced).

| Finding ID | Classification | Evidence | Required action / acceptance | State |
| --- | --- | --- | --- | --- |
| ROM3-R5 | SYSTEM ENTRY UNLOADABLE | `rom_backing_init(apple_rom_bin, ..., 0x8000, ...)` marks system pages occupied. `rom_backing_load_image()` dispatches a SYSTEM entry to `rom_backing_install_region`, whose `range_is_free` rejects all pre-populated pages. Consequently the documented three-entry image containing SYSTEM cannot load via `memory_controller_load_rom_image()`. | Define and implement explicit trusted SYSTEM replacement or preloading policy, or remove SYSTEM from supported packed images; test the real board-startup route with a SYSTEM-containing image. | OPEN |
| ROM3-R6 | INCOMPLETE IMAGE VALIDATION | `rom_backing_load_image` checks magic, table size, payload file end and target region overlap; it **does not** check `version`, declared Apple base, nonzero CRC, or reject payload offsets within the header/table. These fields are specified in `docs/rom-memory-map.md`. | Validate all normative header fields and reject overlapping payload/table/file regions before copying. Add malformed-version, bad-CRC, payload-before-table and mismatched-address negative tests. | OPEN |
| ROM3-R7 | PARTIAL MUTATION ON ERROR | The loader installs entries sequentially. A later invalid entry returns a negative code after earlier entries have already overwritten `rom_space` and marked occupancy; retrying a corrected image may then fail. | Two-pass validation and preflight of all regions/overlaps, then commit all installs (or rollback on error). Test failure after the first valid entry and successful retry. | OPEN |
| ROM3-R8 | PACKING / INTEGRATION LIMIT | `rom_backing_load_image()` exists, but no standalone manifest-to-`*.romimg` packer or automatic startup image selection is in the changed tree. The unit test mirrors the MMU formula rather than invoking the combined Verilator→memory-controller path. | Either implement the minimal format generator and startup fixture in this ticket, or explicitly defer them into the firmware/ROM integration ticket without claiming a complete packaging/CPU-fetch test. | OPEN |

**Verification disposition:** the previous ROM3-R2 runtime blocker is closed by the owner's reported real emulator run and `PR#3/PR#0` test. That is not a ProDOS slot-7 hard-disk boot test. ROM3-R1/R3/R4 improvements are present; ROM3-R5..R8 remain code/format readiness issues.

**Notes**

- **2026-11-21 — Owner directive / DIP32 single-image handoff:** Developer must support loading **the identical raw file sent to the DIP32 programmer** into the emulator. The current `rom_backing_init` + baseline migration + `*.romimg` overlays remain development-only and do not demonstrate raw-image equivalence. Existing owner `PR#3` / `PR#0` verification remains valid for baseline mode, but must be repeated in raw-image mode.

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-20 — Implementation:** Added `emulator/onboard/rom_backing.c`/`.h`: a 64 KiB logical ROM space indexed by the MMU physical address (`MEM_ADDR_OUT`), with system-ROM load, bounds-checked `rom_backing_install` overlays (slot entry / expansion), explicit out-of-bounds reporting (no silent `& 0xffff` wrap) and an optional trace hook (CPU addr, physical addr, ROM_EN_N, byte). `memory_controller.c` now uses it on the ROM path; `rom_memory[ROM_SIZE*2]` and its masking removed.
- **2026-11-20 — Test:** `emulator/onboard/verification/test_rom_backing.c` (24 checks, 0 failures): system ROM read, slot-7 entry overlay at C700–C7FF, slot-7 expansion overlay at C800–CFFE, no aliasing with system ROM, bounds rejection and the trace hook.
- **2026-11-20 — Test:** Full emulator rebuilt via CMake with `rom_backing.c` added — links clean, no regression in the build.
- **2026-11-20 — Decision:** No default slot-7 overlay is wired into `memory_controller_init` so the unmodified Apple ROM boot path is preserved; overlays are installed explicitly once a packaged image exists (`CPLD-ROM-0002` format).
- **2026-11-20 — Status Change:** `OPEN → IN_PROGRESS → REVIEW`; awaiting owner acceptance.

- **2026-11-20 — Independent Code Review:** Reviewed `f77b2605` against direct C unit test and build integration. Keeping the new backing abstraction is reasonable. The advertised image loading/packing and CPU-visible slot7 verification are not yet implemented, and the ticket's 'still boots' acceptance is not supported by a recorded runtime test. The traced production read silently discards its own OOB flag.
- **2026-11-21 — Rework:** Reworked `rom_backing.c`/`.h` for the 4 KiB-per-slot physical layout (0x0000–0x7FFF, 8 windows; system ROM 0x8000+). Added `rom_backing_install_region` (geometry + overlap validation via a 256-byte-page occupancy map), `rom_backing_load_image` (`*.romimg` parser), `rom_backing_fill_baseline`, OOB visibility (`out_of_bounds` trace arg + `rom_backing_oob_count`). `memory_controller.c` now migrates the internal $C100–$CFFF firmware into per-slot windows as a baseline (preserving slot-6 Disk II ROM and boot bytes) and exposes `memory_controller_load_rom_image`.
- **2026-11-21 — Test:** Expanded `test_rom_backing.c` to 45 checks (0 failures): window geometry, validated install + rejections, `*.romimg` load (valid + malformed), OOB on the traced path, and integration reads against the RTL-verified MEM_ADDR_OUT mapping.
- **2026-11-21 — Test:** Full emulator rebuilt clean; runs ~6 s headless (SDL dummy) without crashing. On-screen boot verification deferred (needs display).
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS → REVIEW` for the rework; prior `f77b260` superseded.

- **2026-11-21 — Owner Manual Verification:** Owner reports the emulator runs successfully and that Apple II Applesoft `PR#3` followed by `PR#0` works with the revised 4 KiB slot ROM arrangement. Record as **VERIFIED (OWNER)** for emulator runtime and slot-3/slot-0 transition; this does not imply an IOMEM hard-disk or ProDOS slot7 boot test.
- **2026-11-21 — Independent Re-review:** The producer/consumer physical ROM path is coherent and prior OOB visibility problems are fixed. Owner confirmed the emulator and PR#3/PR#0 work; this resolves the previous runtime-verification gap. Four separate packed-image correctness/completeness findings remain.

**Proposed Change**

### Mandatory DIP32 flash-image integration

Add a raw-flash load mode that seeds the ROM backing array exclusively from the supplied image, not from `apple_rom_bin`, slot-entry copying, expansion replication or later overlays. Read every byte through the normal MMU `ROM_EN_N` + `MEM_ADDR_OUT` path, assert the expected file length and verify byte-for-byte determinism. Raw image composition and complete physical flash acceptance are tracked in ROM-0004.

**Cross-ticket gate:** [`CPLD-ROM-0004`](CPLD-ROM-0004.md) and [single-image contract](../../rom-flash-image-contract.md).

1. Make ROM data selection depend on ROM_EN_N + decoded physical MEM_ADDR_OUT, not direct CPU Cx address shortcuts. Respect the existing system ROM and RAM choices.
2. Implement image loading/packing for slot7 and other configured slots using the reviewed map; ensure physical ROM capacity, bank selection, bounds and ROM holes are explicit, with no silent wrapping by `& 0xffff`.
3. Add emulator-visible traces for CPU address, physical ROM address, ROM_EN_N and returned byte, and unit tests for the C700 and C800 windows.
4. Do not break existing Apple system ROM boot, video, RAM or disk slot6 paths. Keep loading policy compatible with future STM32 ROM access strategy.

### Acceptance criteria and evidence to record

- [x] A synthetic slot7 ROM with distinguishable byte patterns is visible at C700–C7FF and its selected expansion bank at C800–CFFE, without aliasing the system ROM (`test_rom_backing.c` tests 3, 5, 8).
- [x] Cross-slot selections and CFFF release produce the documented bytes; no memory access exceeds loaded image bounds (test 6 OOB on traced path; test 8 CFFF release to slot 0; MMU-side CFFF/slot behaviour verified in `CPLD-ROM-0001` test 4).
- [x] Baseline Apple emulator operates after ROM remapping; owner manually verified the emulator and Applesoft `PR#3` / `PR#0` on 2026-11-21. Developer reports MMU regression tests passed in both builds. This does **not** imply ProDOS slot-7 boot or hardware verification.

### Single-image acceptance gate (added 2026-11-21)

- [ ] The exact single programmer-ready image boots in raw mode and passes repeated `PR#3`/`PR#0` and per-slot ROM regression without runtime overlays.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `f77b260` | `SUPERSEDED` | rom_backing ROM image backing + unit tests; wire into memory_controller |
| `mmu-rom-analysis` | `3c4468c` | `CHANGES_REQUESTED` | 4 KiB/slot rom_backing: validation, loader, OOB visibility, baseline migration |

**Resolution Description**

Awaiting owner review. ROM reads are now served by `rom_backing` from the MMU
physical address with explicit bounds and an optional trace hook; overlays
(slot entry/expansion) are bounds-checked. Verified by `test_rom_backing.c`
(24 checks, 0 failures) and a clean full-emulator rebuild. The default boot
path is unchanged (no overlay installed by default), preserving the Apple
system ROM, video, RAM and slot-6 paths.

**Review disposition (2026-11-20):** Remains `REVIEW` with `CHANGES_REQUESTED` on the implementation commit. The independent review found specific outstanding acceptance criteria; no source changes or test reruns are represented by this review update.

**Rework disposition (2026-11-21):** 4 KiB-per-slot `rom_backing` with validated
installs, a `*.romimg` loader, a documented startup route, OOB visibility and
baseline migration. ROM3-R1/R3/R4 resolved; ROM3-R2 partially met (Apple-visible
bytes preserved by construction, integration fetch asserted, on-screen boot
remains VERIFICATION_REQUIRED). Prior commit `f77b260` superseded. Awaiting owner
re-review/acceptance.
