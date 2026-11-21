# CPLD-ROM-0002 — Specify slot ROM and expansion ROM flash layout

**ID:** `CPLD-ROM-0002`  
**Status:** `REVIEW`\
**Title:** Specify slot ROM and expansion ROM flash layout

**Description**

Specify the 256 KiB emulator ROM map and the raw binary contract for
independently built slot firmware. Define each Apple-visible window and its
array offsets so a slot ROM can be loaded byte for byte.

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping
- Branch baseline: `mmu-rom-analysis` at `ffb534d`
- Dependencies: [CPLD-ROM-0001](CPLD-ROM-0001.md), [CPLD-HW-0001](CPLD-HW-0001.md)

The single array spans `0x00000–0x3FFFF`. Slot windows 1–7 begin at
`slot × 0x1000`. Slot 0 is reserved for a released expansion selection.
The original 32 KiB Apple image is copied as-is to `0x08000–0x0FFFF`.
The standard `$D000–$FFFF` system window and the internal Cx ROM use the
original CPU offsets. The [ROM map](../../rom-memory-map.md) lists each
slot's physical offsets, switch selection and startup API.

Each slot build emits one 4096-byte file. The first 2 KiB contains the
256-byte IOSEL page at `0x000–0x0FF`, then 1792 bytes of padding. The
second 2 KiB contains the IOSTB expansion at `0x800–0xFFF`. Slot 7,
for example, is copied to `0x07000–0x07FFF`, with IOSEL at `0x07000`
and IOSTB at `0x07800`. The slot number is the argument to the loader.
The file consists solely of bytes destined for the slot window.

### Review findings and current evidence

| Finding | Evidence for re-review |
| --- | --- |
| ROM2-R1, overlap and format ambiguity | The combined image uses fixed, disjoint windows. The original Apple file and each slot file are direct byte copies. This supersedes the previous header and entry table. |
| ROM2-R2, expansion extent and release | `$C800–$CFFF` is 2048 bytes, and `$C800–$CFFE` is 2047. The combined bus test checks the last byte at `$CFFF` and the byte after release. With internal Cx selected, `$C800` still reads the Apple ROM. |
| ROM2-R3, usable build and offset contract | The spec lists the base and both mapped ranges for every slot and the output offsets for a separate slot build. The exact-length API copies that file in one operation. The backing test compares all 4096 bytes for every slot. |

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-20 — Implementation:** Authored [`docs/rom-memory-map.md`](../../rom-memory-map.md) specifying Apple CPU windows, the emulator 64 KiB logical ROM space, the physical `*.romimg` flash layout/packing format, and the slot-7 ProDOS reservation.
- **2026-11-20 — Decision:** Grounded the map on the CPLD-ROM-0001 finding that `MEM_ADDR_OUT` equals the raw CPU address under `MODE_SEL=1`; per-slot ROM is addressed by CPU `$Cn00`, and the `$C800–$CFFF` expansion window is a single physical region reserved for slot 7 in the emulator. Hardware multi-card expansion deferred to `CPLD-HW-0001`.
- **2026-11-20 — Status Change:** `OPEN → IN_PROGRESS → REVIEW`; awaiting owner acceptance of the specification.

- **2026-11-20 — Independent Code Review:** Reviewed `docs/rom-memory-map.md` against current RTL and the approved ticket proposal. Image format cannot be implemented as written because the header/table and system ROM payload overlap at offset zero. Also found a 2047-byte C800–CFFE range error and a CFFF mapping ambiguity. Revision needed prior to accepting this as a locked flash layout.
- **2026-11-21 — Rework:** Rewrote `docs/rom-memory-map.md` to rev 2. Added the per-slot 4 KiB window geometry (§2.1), corrected C800–CFFF sizing and CFFF semantics (§2.2), documented C6XX_N bus-ownership (§2.3), redefined the emulator physical ROM space and baseline migration (§3), and fixed the `*.romimg` format with a single `CPRM` magic, post-table payloads, a worked slot-7 example and per-slot flash partitions (§4). Grounded on the rev-2 CPLD-ROM-0001 verified mapping.
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS → REVIEW` for the rework; prior `846dd38` superseded.
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS` on owner direction. Specify a single 256 KiB emulator address space with the unmodified Apple image and one raw 4 KiB binary per slot. The entry page begins at file offset `$000` and expansion begins at `$800`.
- **2026-11-21 — Implementation:** Revised `docs/rom-memory-map.md` for the fixed 256 KiB map, exact 4 KiB raw slot files, the original Apple image and the IOSEL/IOSTB decode. The file byte layout and emulator offsets are now directly related.
- **2026-11-21 — Verification:** The C backing test compares the whole Apple fixture and all seven raw slot fixtures byte for byte. The combined co-simulation reads all 256 slot-7 IOSEL bytes and all 2048 IOSTB bytes, including `$CFFF`, using the real decoder and startup loading route.
- **2026-11-21 — Status Change:** `IN_PROGRESS → REVIEW` with rev 3 and supporting tests ready for owner review.

**Proposed Change**

Review rev 3 of [the ROM map](../../rom-memory-map.md) as the contract for
the emulator and slot firmware build. The hardware ticket owns the physical
CPLD/STM32 ROM service design using these addressed bytes.

- [x] The exact backing size, Apple ROM offset and all seven slot windows are defined.
- [x] The raw file layout, slot startup API and example slot 7 addresses agree with the tested loader.
- [x] IOSEL, IOSTB and CFFF behavior, including internal Apple ROM selection, agree with CPLD-ROM-0001.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `846dd38` | `SUPERSEDED` | Add docs/rom-memory-map.md ROM/expansion flash layout spec |
| `mmu-rom-analysis` | `26f6c06` | `SUPERSEDED` | rom-memory-map.md rev 2: 4 KiB/slot windows, format/CFFF fixes |
| `mmu-rom-analysis` | `1965baa` | `UNREVIEWED` | Record approved raw-ROM scope and start rework |
| `mmu-rom-analysis` | `174966e` | `UNREVIEWED` | Specify the 256 KiB backing and each raw slot binary's file and memory offsets |

**Resolution Description**

Rev 3 documents the raw ROM layout backed by the emulator tests in
CPLD-ROM-0001 and CPLD-ROM-0003. Awaiting owner review. Physical ROM delivery
timing is tracked in CPLD-HW-0001.
