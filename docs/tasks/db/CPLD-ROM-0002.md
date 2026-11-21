# CPLD-ROM-0002 — Specify slot ROM and expansion ROM flash layout

**ID:** `CPLD-ROM-0002`  
**Status:** `REVIEW`\
**Title:** Specify slot ROM and expansion ROM flash layout

**Description**

Specify the compiled ROM inputs and the 256 KiB map shared by the emulator
and target flash. Define each Apple-visible window and its byte offsets.

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping
- Branch baseline: `mmu-rom-analysis` at `ffb534d`
- Dependencies: [CPLD-ROM-0001](CPLD-ROM-0001.md), [CPLD-HW-0001](CPLD-HW-0001.md)

The 256 KiB target image spans `0x00000–0x3FFFF`. It contains
`apple_rom_bin[]` at `0x08000–0x0FFFF` and `slot1_rom_bin[]`,
`slot2_rom_bin[]`, `slot4_rom_bin[]`, `slot5_rom_bin[]`,
`slot6_rom_bin[]` and `slot7_rom_bin[]` at `slot × 0x1000`. Slot 0 and
the external card window for slot 3 are zero-filled. The Apple IIe MMU
selects the internal slot-3 ROM from the Apple image. System ROM at
`$D000–$FFFF` and the other internal Cx reads use their CPU offsets.

Each compiled slot array contains 4096 bytes. The first 2 KiB contains the
256-byte IOSEL page at `0x000–0x0FF`, then 1792 bytes of padding. The second
2 KiB contains IOSTB at `0x800–0xFFF`. Slot 7 is copied to
`0x07000–0x07FFF`, with IOSEL at `0x07000` and IOSTB at `0x07800`.
Each array defaults to zero under its `#ifndef SLOTn_ROM` guard. A supplied
ROM defines the corresponding macro and the array before the fallback.
[ROM map](../../rom-memory-map.md)

### Review findings and current evidence

| Finding | Evidence for re-review |
| --- | --- |
| ROM2-R1, overlap and format ambiguity | Startup copies the compiled Apple and slot arrays into fixed, disjoint windows. The export is the flat physical backing. |
| ROM2-R2, expansion extent and release | `$C800–$CFFF` is 2048 bytes, and `$C800–$CFFE` is 2047. The combined bus test checks the last byte at `$CFFF` and the byte after release. With internal Cx selected, `$C800` still reads the Apple ROM. |
| ROM2-R3, usable build and offset contract | Rev 4 lists the reserved windows, the six symbol/guard pairs and their physical offsets. Compilation enforces the 4096-byte size, and the combined test compares all output bytes with the expected layout. |
| ROM2-R4 and ROM2-R6, example offsets and metadata validation | The raw slot rework supersedes the three-entry example and descriptor validation. All destination ranges are fixed. The release image is a flat file whose offsets are the physical ROM addresses. |
| ROM2-R5, exact CFFF data and phase | The map records the selected slot's `+0xFFF` byte at CFFF and release on the next cycle. ROM1-R4 has direct Verilator assertions and a combined bus test. |

**Notes**

- **2026-11-21 — Owner directive / DIP32 single-image handoff:** The per-slot 4 KiB logical map stays intact, but the deliverable is **one raw DIP32 flash file** whose offsets *are physical ROM addresses*. A `CPRM`/metadata-bearing `*.romimg` is only a development container; it MUST NOT be treated as a programmer-ready image or loaded as a substitute during final emulator verification.

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-20 — Implementation:** Authored [`docs/rom-memory-map.md`](../../rom-memory-map.md) specifying Apple CPU windows, the emulator 64 KiB logical ROM space, the physical `*.romimg` flash layout/packing format, and the slot-7 ProDOS reservation.
- **2026-11-20 — Decision:** Grounded the map on the CPLD-ROM-0001 finding that `MEM_ADDR_OUT` equals the raw CPU address under `MODE_SEL=1`; per-slot ROM is addressed by CPU `$Cn00`, and the `$C800–$CFFF` expansion window is a single physical region reserved for slot 7 in the emulator. Hardware multi-card expansion deferred to `CPLD-HW-0001`.
- **2026-11-20 — Status Change:** `OPEN → IN_PROGRESS → REVIEW`; awaiting owner acceptance of the specification.

- **2026-11-20 — Independent Code Review:** Reviewed `docs/rom-memory-map.md` against current RTL and the approved ticket proposal. Image format cannot be implemented as written because the header/table and system ROM payload overlap at offset zero. Also found a 2047-byte C800–CFFE range error and a CFFF mapping ambiguity. Revision needed prior to accepting this as a locked flash layout.
- **2026-11-21 — Rework:** Rewrote `docs/rom-memory-map.md` to rev 2. Added the per-slot 4 KiB window geometry (§2.1), corrected C800–CFFF sizing and CFFF semantics (§2.2), documented C6XX_N bus-ownership (§2.3), redefined the emulator physical ROM space and baseline migration (§3), and fixed the `*.romimg` format with a single `CPRM` magic, post-table payloads, a worked slot-7 example and per-slot flash partitions (§4). Grounded on the rev-2 CPLD-ROM-0001 verified mapping.
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS → REVIEW` for the rework; prior `846dd38` superseded.

- **2026-11-21 — Owner Manual Verification:** Owner reports the emulator runs successfully and that Apple II Applesoft `PR#3` followed by `PR#0` works with the revised 4 KiB slot ROM arrangement. Record as **VERIFIED (OWNER)** for emulator runtime and slot-3/slot-0 transition; this does not imply an IOMEM hard-disk or ProDOS slot7 boot test.
- **2026-11-21 — Independent Re-review:** Per-slot mapping and flash partition design align with the directive, but the three-entry format example contains overlapping offsets; CFFF's claimed data byte is not asserted; format guarantees exceed what the loader actually validates. Requested targeted spec fixes.
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS` on owner direction. Specify a single 256 KiB emulator address space with the unmodified Apple image and one raw 4 KiB binary per slot. The entry page begins at file offset `$000` and expansion begins at `$800`.
- **2026-11-21 — Implementation:** Revised `docs/rom-memory-map.md` for the fixed 256 KiB map, exact 4 KiB raw slot files, the original Apple image and the IOSEL/IOSTB decode. The file byte layout and emulator offsets are now directly related.
- **2026-11-21 — Verification:** The C backing test compares the whole Apple fixture and all seven raw slot fixtures byte for byte. The combined co-simulation reads all 256 slot-7 IOSEL bytes and all 2048 IOSTB bytes, including `$CFFF`, using the real decoder and startup loading route.
- **2026-11-21 — Status Change:** `IN_PROGRESS → REVIEW` with rev 3 and supporting tests ready for owner review.
- **2026-11-21 — Merge decision:** Kept rev 3 offsets from `174966e`, where the bytes of each raw slot file land in one 4 KiB window. Preserved the one-file DIP32 and emulator parity gate from `22285ea`. The raw layout replaces the container work requested in ROM2-R4/R6, and ROM2-R5 is verified by the exact CFFF test. The chip-size, fill and readback decision remains in CPLD-ROM-0004 and CPLD-HW-0001.
- **2026-11-21 — Owner directive and status change:** `REVIEW → IN_PROGRESS` to document the compiled Apple and slot arrays. Slots 1, 2, 4, 5, 6 and 7 have 4096-byte inputs. Slots 0 and 3 have no slot input. The full 256 KiB backing is the target ROM image.
- **2026-11-21 — Verification and status change:** `IN_PROGRESS → REVIEW` for rev 4. The default and overridden builds emit the exact documented Apple, slot and fill bytes in a 262144-byte file. The separately compiled slot-6 fixture begins at `0x06000`, and selecting internal slot 3 still uses `apple_rom_bin[]`.

**Proposed Change**

Review rev 4 of [the ROM map](../../rom-memory-map.md) as the contract for
the compiled inputs and `-w` output. The hardware ticket owns the ROM bus
and programming verification.

- [x] The exact target size, Apple ROM offset, six compiled slot offsets and empty slot 0 and 3 windows are defined.
- [x] The symbol and guard names, IOSEL/IOSTB offsets and default fill agree with the compiled image and its file export.
- [x] IOSEL, IOSTB and CFFF behavior, including internal Apple ROM selection, agree with CPLD-ROM-0001.
- [ ] Final programmer image size, high-bank fill and device fit are verified on the chosen part. [CPLD-ROM-0004](CPLD-ROM-0004.md)

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `846dd38` | `SUPERSEDED` | Add docs/rom-memory-map.md ROM/expansion flash layout spec |
| `mmu-rom-analysis` | `26f6c06` | `SUPERSEDED` | rom-memory-map.md rev 2: 4 KiB/slot windows, format/CFFF fixes |
| `mmu-rom-analysis` | `2cdcba6` | `CHANGES_REQUESTED` | Independent re-review of `26f6c06`, with ROM2-R4 through ROM2-R6 |
| `mmu-rom-analysis` | `22285ea` | `UNREVIEWED` | Record the single-image DIP32 and emulator acceptance gate |
| `mmu-rom-analysis` | `1965baa` | `UNREVIEWED` | Record approved raw-ROM scope and start rework |
| `mmu-rom-analysis` | `174966e` | `UNREVIEWED` | Specify the 256 KiB backing and each raw slot binary's file and memory offsets |
| `mmu-rom-analysis` | `8715356` | `UNREVIEWED` | Reconcile rev 3 offsets with the single-file release contract |
| `mmu-rom-analysis` | `562ce53` | `UNREVIEWED` | Record the compiled inputs and start rev 4 |
| `mmu-rom-analysis` | `8df4a4f` | `UNREVIEWED` | Document rev 4 and verify the compiled inputs at their target offsets |

**Resolution Description**

Rev 4 documents the compiled sources, byte offsets, reserved windows and
256 KiB export. Physical ROM timing remains in CPLD-HW-0001.
