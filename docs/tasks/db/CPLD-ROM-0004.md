# CPLD-ROM-0004 — Produce and emulate one programmer-ready DIP32 flash image

**ID:** `CPLD-ROM-0004`  
**Status:** `OPEN`  
**Title:** Produce and emulate one programmer-ready DIP32 flash image

**Description**

The DIP32 hardware ROM takes one contiguous headerless image. Build that file
deterministically, verify it by booting the emulator directly from its bytes,
and compare full-chip programmer readback with the tested file.

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping; hardware-ready acceptance gate for M2/M4
- Branch baseline: `mmu-rom-analysis`, 2026-11-21
- Dependencies: [`CPLD-ROM-0001`](CPLD-ROM-0001.md), [`CPLD-ROM-0002`](CPLD-ROM-0002.md), [`CPLD-ROM-0003`](CPLD-ROM-0003.md) and [`CPLD-HW-0001`](CPLD-HW-0001.md) for actual part/pinout/capacity
- Binding architectural reference: [`docs/rom-flash-image-contract.md`](../../rom-flash-image-contract.md)
- This ticket **does not** authorise additional CPLD SPI-master RTL or relocate RTC/STM32 serial-flash responsibilities.

### Fixed logical ROM address map

The current MMU uses the lowest 64 KiB of the 256 KiB emulator ROM
backing. File offsets follow the physical byte addresses from ROM map rev 3.

- `0x0000–0x7FFF`: eight 4 KiB slot-index regions (slot 0 reserved; slots 1–7).
- Slot `N` entry ROM: `N << 12`, the first 256 bytes of its window (IOSEL).
- Slot `N` expansion ROM: `(N<<12)|0x800`, 2048 bytes in second 2 KiB (IOSTB).
- Slot 6 card entry, when supplied, starts at `0x06000`. Slot 7 entry starts at `0x07000` and expansion at `0x07800`.
- `0x08000–0x0FFFF`: unmodified Apple image, including the original internal `$C600–$C6FF` and system `$D000–$FFFF` offsets.
- `0x10000–0x3FFFF`: reserved emulator capacity. Define high-address contents for the confirmed device.

**Important:** DIP32 specifies the package only. Confirm part number, actual capacity/address lines, CE/OE/WE and programmer full-chip image size in `CPLD-HW-0001`; the 64 KiB decoded layout is not proof of a 64 KiB flash chip. A larger device still receives **one** deterministic full-chip image when required.

### Deliverables and constraints

1. A documented deterministic build/packing tool for one **headerless** physical programmer binary (suggested `apple_cpld_flash.bin`), with independent map/manifest/hash as optional sidecars. Never program `CPRM`/descriptor metadata at raw ROM offset 0.
2. Compose the image from the original Apple bytes and one raw 4096-byte input for each populated slot. Record the fill rule and device-size or high-bank policy separately.
3. Start the emulator with the exact raw programmer file as its sole ROM content. Normal `MEM_ADDR_OUT`, `ROM_EN_N` and slot strobes drive the emulator fetches. The direct startup loader is tracked in CPLD-ROM-0003.
4. Regression tests comparing raw file bytes with both direct physical ROM reads and real MMU→memory bus CPU-visible fetches; verify slot boundaries, slot 6 legacy ROM, slot 7 firmware, per-slot C800 expansion, CFFF semantics and system ROM vectors. Avoid treating an independent copy of the RTL formula as a full integration test.
5. Rebuild deterministically twice, verify identical SHA-256; boot the emulator with the raw image and verify `PR#3`/`PR#0`, floppy paths and (when available) slot7 ProDOS/IOMEM. The owner has already confirmed `PR#3`/`PR#0` for the older reconstructed ROM; **repeat that test in raw mode**.
6. Document hardware programming/readback procedure and verify complete byte-for-byte readback against the very file tested in the emulator. If the chip is unavailable, mark this subtest **PENDING HARDWARE VERIFICATION**, not complete.

**Notes**

- **2026-11-21 — Owner Directive:** Hardware ROM must consist of **one image written to the DIP32 flash**. The emulator must reproduce that same image exactly to establish target-device confidence. This supersedes any assumption that per-slot development containers or RAM overlays are the final physical flash format.
- **2026-11-21 — Design Boundary:** The STM32 still acts as SPI master and owns RTC/serial flash. The DIP32 hardware ROM is a separate, directly addressed hardware resource. Do not conflate it with STM32-side SPI storage.
- **2026-11-21 — Baseline Test Evidence:** The owner reports the emulator works and `PR#3` / `PR#0` pass with the developer's slot-ROM remapping; this verifies the existing emulator mode, **not** a programmer-ready raw image.
- **2026-11-21 — Merge decision:** Retained the one-image release and chip readback gates from `22285ea`. The approved raw slot binaries from `174966e` use offset zero for IOSEL, so the earlier `0x6600` and `0x7700` entry offsets are superseded. Apple internal Cx bytes keep their original offsets. The part number, lower IOSEL address wiring, device size/fill policy, reproducible release composition and tests on the actual file remain open.
- **2026-11-21 — Emulator prerequisite:** CPLD-ROM-0003 added `--rom IMAGE.bin` and verified byte parity, `PR#3`/`PR#0` and the Disk II bootstrap with 256 KiB fixtures. Repeat these checks with the reproducibly composed programmer file after the part and its file length are confirmed.

**Proposed Change**

Implement the deterministic release composer against the addresses in the ROM
map and use the raw startup path from CPLD-ROM-0003. Confirm the part before
locking the device file size, high address-line policy and fill.
[DIP32 flash-image contract](../../rom-flash-image-contract.md)

### Acceptance criteria

- [ ] One single headerless, deterministic programmer-ready raw ROM image exists and every logical ROM address is explicit, with no hidden remapping or overlays.
- [ ] Full device capacity and high-bank/padding policy match a specified, verified DIP32 part; output is exactly usable by the selected programmer.
- [ ] Emulator accepts the exact binary as its **sole ROM content** and produces byte-for-byte identical reads at all physical addresses in the current mapping.
- [ ] End-to-end MMU/ROM fetch test, boot and `PR#3`/`PR#0` regression use the final binary in raw-flash mode.
- [ ] Raw image digest, input manifest, output length, source commit and test results are recorded; rebuilding yields identical binary.
- [ ] Programming and complete-chip readback match the same file byte-for-byte. If chip/programmer is not yet available, this item remains PENDING HARDWARE VERIFICATION.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `22285ea` | `UNREVIEWED` | Owner release requirements and the DIP32 hardware acceptance gate |
| `mmu-rom-analysis` | `8715356` | `UNREVIEWED` | Adopt the rev 3 physical byte offsets and keep final chip validation open |

**Resolution Description**

Pending release composition, confirmed chip parameters, emulator validation of
the final file and complete programmer readback.
