# CPLD-ROM-0003 — Implement emulator ROM image backing from ROM_EN_N and MEM_ADDR_OUT

**ID:** `CPLD-ROM-0003`  
**Status:** `REVIEW`\
**Title:** Implement emulator ROM image backing from ROM_EN_N and MEM_ADDR_OUT

**Description**

Serve the original Apple ROM and separately built card ROMs from one
256 KiB array. Load Apple firmware at its original offsets and provide an
API that copies a raw 4 KiB binary into the requested slot.

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping
- Branch baseline: `mmu-rom-analysis` at `ffb534d`
- Dependencies: [CPLD-ROM-0001](CPLD-ROM-0001.md), [CPLD-ROM-0002](CPLD-ROM-0002.md)

`memory_controller_init()` clears the backing and copies the original 32 KiB
Apple image to `0x08000`. The MMU selects the original bytes for
system ROM at `$D000–$FFFF` and for internal Cx reads. The slot windows
start empty. `memory_controller_load_slot_rom(slot, bytes, length)` copies
one 4096-byte binary verbatim to `slot × 0x1000` after initialization
and before CPU execution. IOSEL is at file offset `$000` and IOSTB is at
`$800`. Valid repeat loads replace the entire slot. Invalid input leaves
the array intact. [ROM map](../../rom-memory-map.md)

`build/apple_cpld --rom IMAGE.bin` initializes the board from an exact
262144-byte flat file. The backing copies its bytes in physical address order
after a complete read and length check. Raw mode rejects subsequent slot
loads. The DIP32 device and programmer file length are tracked separately in
[CPLD-ROM-0004](CPLD-ROM-0004.md).

Normal board reads use the address sampled from `MEM_ADDR_OUT`, with
`ROM_EN_N` for the Apple ROM and IOSEL, C6XX or IOSTB for an external slot
in emulator mode. The shared backing traces the original CPU address,
physical address, sampled `ROM_EN_N`, returned byte and bounds status.

### Review findings and current evidence

| Finding | Evidence for re-review |
| --- | --- |
| ROM3-R1, CPU-visible startup loading | The combined test initializes the board's memory controller, loads raw slot-6 and slot-7 fixtures through the public API and reads their bytes through the actual Verilator scheduler and controller. |
| ROM3-R2, real startup and integration | The combined model checks cross-slot selection and the `$CFFF` byte and release. The full emulator reached the Apple IIe prompt and executed the slot-6 disk bootstrap with a raw fixture. |
| ROM3-R3, bounds visibility | The backing bounds check uses `0x40000`. A bus-test injection at that address through `memory_controller_access()` returns zero, raises the trace flag and increments the count. An RTL-generated address is at most 17 bits. |
| ROM3-R4, input validation and isolation | The loader validates slots 1–7, a non-null pointer and exact 4096-byte length before one copy. The C tests cover short, long, overflowing and invalid-slot inputs, repeat loads and system/neighbor byte isolation. |
| ROM3-R5 and ROM3-R6, system entry and header validation | The flat file replaces the tagged container and supplies every byte, including the Apple ROM. The startup loader checks the exact length and the complete read. The tests compare all 262144 bytes. |
| ROM3-R7, partial mutation | Both loaders validate before copying. Short and long file rejection, preserved contents and mode after failure, and a successful retry are tested. |
| ROM3-R8, packaging and integration | The real combined bus tests both startup modes. The actual desktop app boots from flat fixtures. Deterministic release composition and the final chip parity test remain in CPLD-ROM-0004. |

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
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS` on owner direction to load the Apple image and raw 4 KiB slot binaries into a single 256 KiB array. The current 64 KiB backing, Cx ROM replication and packed-image loader are the baseline under rework.
- **2026-11-21 — Discovery:** The baseline unit test passes 45 checks. Its integration assertions reproduce expected hardware addresses in C. Verification will drive the combined IOU/MMU model through the production scheduler and memory controller and exercise Apple startup with the original ROM bytes.
- **2026-11-21 — Implementation:** `rom_backing.c` and `rom_backing.h` replace the tagged loader, region overlays and synthetic Cx copies with one 256 KiB backing and a fixed-layout raw slot copy. `memory_controller.c` and `memory_controller.h` expose the slot API and serve Apple and card ROM from captured MMU addresses. The board loads the existing Apple image once at startup.
- **2026-11-21 — Verification:** `bash emulator/onboard/verification/run_tests.sh` passes 39 backing checks and 18,464 combined `IOU_MMU_tb` bus checks per variant (Verilator 5.045, GCC 15.2.1). The bus test compares 16,128 original Apple ROM bytes at `$C100–$FFFF` with the embedded image, in addition to slot dumps, cross-slot/CFFF tests, disk I/O decode and a real language-card switch sequence.
- **2026-11-21 — Runtime verification:** Regenerated the default combined RTL model and built the desktop emulator with CMake 3.31.6 on Fedora 42 (GCC 15.2.1, SDL 2.33.0). Captured the SDL software-rendered framebuffer using the dummy video driver. With the original Apple image and all card slots empty, both the baseline and the reworked emulator displayed `Apple //e` and the Applesoft `]` prompt within five seconds.
- **2026-11-21 — Disk II verification:** A temporary startup probe put the project's 256 `disc_rom_access` bytes at offset zero of a 4096-byte slot-6 file buffer and loaded it through `memory_controller_load_slot_rom`. With a scratch copy of `solo-flight.dsk`, the CPU fetched the bootstrap at `$C600`, loaded all 256 sector-zero bytes into `$0800–$08FF` byte for byte, then fetched its first instruction at `$0801` from the loaded sector.
- **2026-11-21 — Status Change:** `IN_PROGRESS → REVIEW` after combined bus, Apple ROM startup and Disk II bootstrap verification.
- **2026-11-21 — Merge decision:** Preserved the single 256 KiB backing, the raw slot API, and the combined regression from `174966e`. The owner runtime report in `2cdcba6` applies to the older development setup. The tagged-format findings ROM3-R5 through ROM3-R7 are superseded for slot loads. The required whole-image startup and raw-mode tests from `22285ea` remain in scope here. Release image composition and physical readback remain in CPLD-ROM-0004.
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS` to implement direct startup from a flat raw ROM while retaining the development slot API.
- **2026-11-21 — Implementation:** `source/app_emulator.c` adds `--rom IMAGE.bin`. `board.c` and `memory_controller.c` select the file at startup. `rom_backing.c` reads exactly 262144 bytes before committing the copy and rejects slot writes in raw mode. Before the change an invalid `--rom` path started the default emulator. Missing, short and oversized files now exit with a load error.
- **2026-11-21 — Verification:** `bash emulator/onboard/verification/run_tests.sh` passes 55 backing checks and 50,730 combined bus checks per variant (Verilator 5.045, GCC 15.2.1). Raw-mode probes compare all seven IOSEL and IOSTB windows, their CFFF bytes and subsequent release, and the original address ranges for internal Cx and system ROM against fixture file bytes. `bash rtl_MMU/verification/run_tests.sh` passes 462 checks per variant.
- **2026-11-21 — Raw desktop verification:** CMake 3.31.6, Fedora 42, SDL 2.33.0 software renderer with dummy video. Direct startup with the Apple-only image reached the `Apple //e` Applesoft prompt. A temporary probe fed `PR#3`, `PR#0` and `PRINT 2+3` through the keyboard controller's normal SPI path and captured the rendered screen. The CPU entered internal slot 3 at `$C300`, then the screen showed `PR#0`, the result `5` and a new `]` prompt. The fixture used the original Apple image at `0x08000`, zero fill and SHA-256 `4f598d9a529f1a65fa83e8674b0c41c9950d60c1843839da788fd96ce0ffe7f0`.
- **2026-11-21 — Raw Disk II verification:** A second flat fixture placed the existing 256-byte Disk II bootstrap at `0x06000` and had SHA-256 `a73675446e4eebf0d9f1eb4c29881e16e416a7dc56755847c08ed0712cdb14f0`. The CPU fetched it at `$C600`, copied all 256 sector-zero bytes from a scratch copy of `solo-flight.dsk` and fetched the first sector instruction at `$0801`. Both desktop probes used the raw CLI with the fixture bytes fixed before startup.
- **2026-11-21 — Status Change:** `IN_PROGRESS → REVIEW` for direct raw-file startup and fixture verification.

**Proposed Change**

Supply the shared backing, direct slot-load API and phase-sampled access tests.
Use raw firmware bytes as specified in [CPLD-ROM-0002](CPLD-ROM-0002.md).
Add a raw-file startup mode that uses the supplied bytes as the sole ROM
content and rejects later slot copies in that mode.

- [x] A synthetic 4 KiB slot-7 binary reads byte for byte at `$C700–$C7FF` and `$C800–$CFFF`.
- [x] Cross-slot access, CFFF release, disabled ROM writes, invalid inputs and traced bounds behavior pass.
- [x] Full emulator reaches the Apple ROM prompt with the unmodified image and empty slot windows.
- [x] Complete flat fixtures boot directly and provide byte-exact MMU reads. Repeated `PR#3`/`PR#0` and verified the Disk II bootstrap in raw mode.
- [ ] Test the actual programmer image after composition and DIP32 part confirmation. [CPLD-ROM-0004](CPLD-ROM-0004.md)

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `f77b260` | `SUPERSEDED` | rom_backing ROM image backing + unit tests; wire into memory_controller |
| `mmu-rom-analysis` | `3c4468c` | `SUPERSEDED` | 4 KiB/slot rom_backing: validation, loader, OOB visibility, baseline migration |
| `mmu-rom-analysis` | `2cdcba6` | `CHANGES_REQUESTED` | Independent re-review of `3c4468c`, including ROM3-R5 through ROM3-R8 |
| `mmu-rom-analysis` | `22285ea` | `UNREVIEWED` | Add the same-file raw startup acceptance gate |
| `mmu-rom-analysis` | `1965baa` | `UNREVIEWED` | Record approved raw-ROM scope and start rework |
| `mmu-rom-analysis` | `174966e` | `UNREVIEWED` | Add raw ROM backing and slot-load API with combined bus and startup verification |
| `mmu-rom-analysis` | `8715356` | `UNREVIEWED` | Reconcile the reviewer findings and carry forward raw-file startup |

**Resolution Description**

The 256 KiB array, development slot loading and direct raw-file startup passed
backing, combined bus and desktop fixture verification. Awaiting owner review.
Final programmer-image verification remains in CPLD-ROM-0004. Slot-7 ProDOS
boot and IOMEM disk transfers remain in [CPLD-TEST-0001](CPLD-TEST-0001.md).
