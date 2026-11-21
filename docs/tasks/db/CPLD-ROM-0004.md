# CPLD-ROM-0004 — Produce and emulate one programmer-ready flash image

**ID:** `CPLD-ROM-0004`  
**Status:** `REVIEW`\
**Title:** Produce and emulate one programmer-ready flash image

**Description**

The SST39VF010-70 takes one contiguous, headerless 128 KiB image. The
emulator builds it in memory from the compiled ROM arrays, optionally exports
those bytes during startup and runs from the same backing. The lower 64 KiB
preserves the approved layout. [128 KiB retarget](CPLD-ROM-0005.md)

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping; hardware-ready acceptance gate for M2/M4
- Branch baseline: `mmu-rom-analysis`, 2026-11-21
- Dependencies: [`CPLD-ROM-0001`](CPLD-ROM-0001.md), [`CPLD-ROM-0002`](CPLD-ROM-0002.md), [`CPLD-ROM-0003`](CPLD-ROM-0003.md), [`CPLD-ROM-0005`](CPLD-ROM-0005.md) for the retarget and [`CPLD-HW-0001`](CPLD-HW-0001.md) for the physical interface
- Binding architectural reference: [`docs/rom-flash-image-contract.md`](../../rom-flash-image-contract.md)
- This ticket **does not** authorise additional CPLD SPI-master RTL or relocate RTC/STM32 serial-flash responsibilities.

### Fixed logical ROM address map

The current MMU uses the lower 64 KiB of the 128 KiB emulator ROM
backing. File offsets follow the physical byte addresses from ROM map rev 5.

- `0x0000–0x7FFF`: eight 4 KiB slot-index regions. Slots 0 and 3 are zero-filled. Slots 1, 2, 4, 5, 6 and 7 use the compiled arrays.
- Slot `N` entry ROM: `N << 12`, the first 256 bytes of its window (IOSEL).
- Slot `N` expansion ROM: `(N<<12)|0x800`, 2048 bytes in second 2 KiB (IOSTB).
- Slot 6 card entry, when supplied, starts at `0x06000`. Slot 7 entry starts at `0x07000` and expansion at `0x07800`.
- `0x08000–0x0FFFF`: unmodified Apple image, including the original internal `$C600–$C6FF` and system `$D000–$FFFF` offsets.
- `0x10000–0x1FFFF`: zero-filled reserved bank. The target holds flash A16 low.

Confirm the package suffix, adapter or footprint, voltage domains, address
lines and CE#/OE#/WE# before programming. The full readback covers both
64 KiB banks. [CPLD-HW-0001](CPLD-HW-0001.md)

### Deliverables and constraints

1. Compile `apple_rom_bin[]` and the six 4096-byte slot arrays. Document the source revision, populated slots and their `SLOTn_ROM` definitions.
2. Start the emulator with `-w IMAGEFILE` to write exactly 131072 bytes from the populated backing. Startup without arguments creates no file.
3. Compare the exported bytes with the compiled inputs, whole backing and real MMU-driven reads. Probe the empty slots, IOSEL/IOSTB, CFFF, Apple internal ROM and system vectors.
4. Compare independent builds and SHA-256. Verify `PR#3`/`PR#0` and the floppy bootstrap during a run that writes the image. Repeat with the populated release build including slot-7 ProDOS/IOMEM when available.
5. Program the exported file and compare the full programmer readback byte for byte. Record the part, wiring, programmer settings and tested image digest.

### Independent re-review — 2026-11-21 (approved 256 KiB baseline)

The reviewer approved the 262144-byte exporter built from compiled ROMs
and the optional output-only CLI at `f4da91d`. Evidence included repeatable
default and release exports, compiled slot overrides, deterministic reserved
regions and partial-write handling. The owner verified startup, Applesoft
`PR#3`/`PR#0` and the compiled slot patterns. Physical readback and the
populated slot-7 release remained open. ROM-0005 changes the flash capacity
and verifies lower-bank parity with this approved baseline.

**Notes**

- **2026-11-21 — Owner Directive:** Hardware ROM must consist of **one image written to the DIP32 flash**. The emulator must reproduce that same image exactly to establish target-device confidence. This supersedes any assumption that per-slot development containers or RAM overlays are the final physical flash format.
- **2026-11-21 — Design Boundary:** The STM32 still acts as SPI master and owns RTC/serial flash. The DIP32 hardware ROM is a separate, directly addressed hardware resource. Do not conflate it with STM32-side SPI storage.
- **2026-11-21 — Baseline Test Evidence:** The owner reports the emulator works and `PR#3` / `PR#0` pass with the developer's slot-ROM remapping; this verifies the existing emulator mode, **not** a programmer-ready raw image.
- **2026-11-21 — Merge decision:** Retained the one-image release and chip readback gates from `22285ea`. The approved raw slot binaries from `174966e` use offset zero for IOSEL, so the earlier `0x6600` and `0x7700` entry offsets are superseded. Apple internal Cx bytes keep their original offsets. The part number, lower IOSEL address wiring, device size/fill policy, reproducible release composition and tests on the actual file remain open.
- **2026-11-21 — Emulator prerequisite:** CPLD-ROM-0003 added `--rom IMAGE.bin` and verified byte parity, `PR#3`/`PR#0` and the Disk II bootstrap with 256 KiB fixtures. Repeat these checks with the reproducibly composed programmer file after the part and its file length are confirmed.
- **2026-11-21 — Owner directive and status change:** `OPEN → IN_PROGRESS` to replace the separate composition and raw-file boot workflow. The emulator assembles the 256 KiB target image from compiled inputs and emits the exact backing via `-w IMAGEFILE` after loading all arrays. The previous CLI rejected `-w` and created no image. Verify byte parity, optional output and boot against the new entry point.
- **2026-11-21 — Export verification:** The default debug and independent release builds exported identical 262144-byte files with SHA-256 `4f598d9a529f1a65fa83e8674b0c41c9950d60c1843839da788fd96ce0ffe7f0`. The Disk II fixture build with `SLOT6_ROM` exported 262144 bytes with SHA-256 `a73675446e4eebf0d9f1eb4c29881e16e416a7dc56755847c08ed0712cdb14f0`. Both matched the documented layout byte for byte. A no-argument run created no file. The CLI rejected the former input option and incomplete `-w` requests. An export under a 1024-byte file size limit preserved the existing destination and removed its temporary output.
- **2026-11-21 — Status Change:** `IN_PROGRESS → REVIEW` for the target image exporter. The populated release, DIP32 wiring and hardware readback gates are still open.

- **2026-11-21 — Independent Acceptance Review:** Approved software export and no-input-CLI implementation. Do not mark complete until actual DIP32 part/capacity/addressing, programmer readback and populated ProDOS/IOMEM tests; these are explicitly retained, not defects in current emulator code.
- **2026-11-21 — Review Status:** Remains `REVIEW` because hardware/programming and populated slot7 acceptance items are not yet complete.

- **2026-11-21 — Owner scope change:** SST39VF010-70 (128 KiB) replaces the 256 KiB flash-size assumption. CPLD-ROM-0005 owns emulator resize/export/tests and required physical-voltage/package review. Preserve prior 256 KiB development evidence in historical notes; new ROM-0004 acceptance image size must follow ROM-0005 once implemented. Keep physical programming/readback pending.
- **2026-11-21 — Flash contract update:** The 131072-byte export preserves the approved lower-bank bytes and zero-fills the reserved upper bank. The reference connections and timing are recorded in CPLD-HW-0001. Proceed to review with the software evidence in CPLD-ROM-0005. Physical programming/readback and the populated slot-7 acceptance gates remain open.

**Proposed Change**

Use the emulator's `-w` export of its compiled startup image as the target
programmer file. Verify the selected package, lower-bank wiring and full
chip readback. [Flash image contract](../../rom-flash-image-contract.md)

### Acceptance criteria

- [x] `-w IMAGEFILE` writes the single headerless 128 KiB backing, and a no-argument run creates no file.
- [x] Default and overridden builds export the expected bytes. The MMU fetch tests and desktop boot probes use the backing emitted in their run.
- [x] The default image reproduces across separate builds. Fixture lengths, hashes and test results are recorded.
- [ ] The SST39VF010-70 package, address and data wiring, and programmer are confirmed for the exact 128 KiB file.
- [ ] The populated release build passes its slot-7 ProDOS and IOMEM acceptance tests. [CPLD-TEST-0001](CPLD-TEST-0001.md)
- [ ] Programming and complete chip readback match the exported file byte for byte.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `22285ea` | `UNREVIEWED` | Owner release requirements and the DIP32 hardware acceptance gate |
| `mmu-rom-analysis` | `8715356` | `UNREVIEWED` | Adopt the rev 3 physical byte offsets and keep final chip validation open |
| `mmu-rom-analysis` | `5184c00` | `SUPERSEDED` | Emulator prerequisite for direct raw-file tests with the programmer image |
| `mmu-rom-analysis` | `562ce53` | `APPROVED` | Record direct export of the compiled startup image |
| `mmu-rom-analysis` | `8df4a4f` | `APPROVED` | Export the populated backing with `-w` and record byte parity and boot evidence |
| `mmu-rom-analysis` | `cf47ae7` | `UNREVIEWED` | Apply the 128 KiB export and readback contract from CPLD-ROM-0005 |

**Resolution Description**

The compiled-ROM workflow was approved on 2026-11-21. The 128 KiB retarget
and regression evidence are in [CPLD-ROM-0005](CPLD-ROM-0005.md) for review.
This ticket stays in REVIEW for physical programming and complete readback
and for the populated slot-7 ProDOS/IOMEM release.
