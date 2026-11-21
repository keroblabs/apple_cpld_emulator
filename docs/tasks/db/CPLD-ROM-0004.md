# CPLD-ROM-0004 — Generate and verify one flash image in the emulator

**ID:** `CPLD-ROM-0004`  
**Status:** `RESOLVED`  
**Title:** Generate and verify one flash image in the emulator

**Description**

The SST39VF010-70 takes one contiguous, headerless 128 KiB image. The
emulator builds it in memory from the compiled ROM arrays, optionally exports
those bytes during startup and runs from the same backing. The lower 64 KiB
preserves the approved layout. [128 KiB retarget](CPLD-ROM-0005.md)

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping; compiled ROM image generation and software verification
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
4. Compare independent builds and SHA-256. Verify `PR#3`/`PR#0` and the floppy bootstrap during a run that writes the image.
5. **External acceptance:** the populated slot-7 ProDOS/IOMEM release test is owned by [CPLD-TEST-0001](CPLD-TEST-0001.md), while physical programming and complete chip readback are owned by [CPLD-HW-0001](CPLD-HW-0001.md). These are not part of ROM-0004's software-resolution gate.

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

- **2026-11-21 — Independent review and scope split:** Source-reviewed the compiled ROM backing, `-w` export path, checked 128 KiB size and accompanying tests. The developer recorded repeatable full-file comparisons and emulator boot/slot-pattern tests; owner also verified `PR#3`/`PR#0` and per-slot ROM patterns in the preceding configuration. No source-code rework requested. **Approved the software image composition and export deliverable.** Transferred physical flash programming, device/package/voltage/wiring/timing and complete-chip readback to CPLD-HW-0001, and populated slot7 ProDOS/IOMEM to CPLD-FW-0001 and CPLD-TEST-0001. This is a scope clarification, not a claim those tests passed.
- **2026-11-21 — Status Change:** `REVIEW → RESOLVED` for the emulator image-export deliverable; requires owner merge/integration before CLOSED.

**Proposed Change**

Use the emulator's `-w` export of its compiled startup image as the target
programmer file. Verify the selected package, lower-bank wiring and full
chip readback. [Flash image contract](../../rom-flash-image-contract.md)

### Software acceptance criteria (scope resolved 2026-11-21)

- [x] The emulator's `-w IMAGEFILE` exports the one headerless **131072-byte** binary from its compiled startup backing; a no-argument run does not create a ROM file.
- [x] Default and all six overridden slot builds produce byte-accurate exports and execute from those very same compiled bytes, including IOSEL, IOSTB, CFFF and original Apple ROM.
- [x] Independent-build exports are reproducible, source-byte and whole-file comparisons pass, and a forced short write leaves the prior export intact.
- [x] Existing Apple IIe boot, `PR#3`/`PR#0`, slot-pattern and slot6 Disk II bootstrap developer tests are recorded, with earlier owner manual verification preserved.
- [x] ROM-0005 retarget results are accepted: 128 KiB output, first 64 KiB parity, 64 KiB reserved zero fill, no new ROM bank select.

### External hardware and firmware acceptance (transferred, not completed)

- [ ] **CPLD-HW-0001:** confirm the SST39VF010-70 exact suffix/PLCC32 or TSOP32 adapter or footprint, 3.3 V electrical interface, address mux and A16 strap, timing, CE#/OE#/WE#, programming and **full 131072-byte chip readback** against the exported hash.
- [ ] **CPLD-FW-0001 / CPLD-TEST-0001:** implement populated slot7 ProDOS firmware, IOMEM disk service, and end-to-end boot/512-byte block tests using the corresponding `-w` output.

**Resolution boundary:** ROM-0004 produces a deterministic **programmer-input file**, not evidence that a physical flash has yet been programmed or that the future ProDOS firmware runs. Hardware/firmware obligations have not been waived; their owning tickets remain OPEN.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `22285ea` | `UNREVIEWED` | Owner release requirements and the DIP32 hardware acceptance gate |
| `mmu-rom-analysis` | `8715356` | `UNREVIEWED` | Adopt the rev 3 physical byte offsets and keep final chip validation open |
| `mmu-rom-analysis` | `5184c00` | `SUPERSEDED` | Emulator prerequisite for direct raw-file tests with the programmer image |
| `mmu-rom-analysis` | `562ce53` | `APPROVED` | Record direct export of the compiled startup image |
| `mmu-rom-analysis` | `8df4a4f` | `APPROVED` | Export the populated backing with `-w` and record byte parity and boot evidence |
| `mmu-rom-analysis` | `cf47ae7` | `APPROVED` | Apply the 128 KiB export and readback contract from CPLD-ROM-0005 |

**Resolution Description**

**RESOLVED (software), 2026-11-21.** The compiled-in Apple/slot ROM arrays initialise the same 128 KiB backing from which the emulator executes, and `-w IMAGEFILE` exports precisely those bytes in one reproducible programmer-input file. The existing `PR#3`/`PR#0`, slot-pattern, Disk II and export-parity evidence was reviewed and accepted. The original 256 KiB results are preserved in Notes as historical evidence; the approved target image is now 128 KiB. No implementation changes were necessary in this review.

**Excluded from this resolution:** actual SST39VF010-70 purchase/package/adapter verification, electrical and timing validation, flash programming and **131072-byte readback** (CPLD-HW-0001), and the *future populated* ProDOS/IOMEM image integration (CPLD-FW-0001 / CPLD-TEST-0001). These gates are still open and must not be reported as tested. Ticket remains pending merge/integration for CLOSED.
