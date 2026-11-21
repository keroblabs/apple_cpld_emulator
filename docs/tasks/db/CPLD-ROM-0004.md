# CPLD-ROM-0004 — Produce and emulate one programmer-ready DIP32 flash image

**ID:** `CPLD-ROM-0004`  
**Status:** `OPEN`  
**Title:** Produce and emulate one programmer-ready DIP32 flash image

**Description**

The final hardware ROM is **one physical DIP32 flash IC programmed from a single contiguous raw image**. Current `rom_backing` builds ROM in memory by loading `apple_rom_bin`, migrating per-slot Cn00 pages and C800 baselines, and optionally applying `*.romimg` regions. That is useful for development but does **not** prove that a single programmer image can reproduce the emulated machine. The developer must implement one deterministic physical image and run the emulator directly from those very same bytes before the image is committed to real hardware.

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping; hardware-ready acceptance gate for M2/M4
- Branch baseline: `mmu-rom-analysis`, 2026-11-21
- Dependencies: [`CPLD-ROM-0001`](CPLD-ROM-0001.md), [`CPLD-ROM-0002`](CPLD-ROM-0002.md), [`CPLD-ROM-0003`](CPLD-ROM-0003.md) and [`CPLD-HW-0001`](CPLD-HW-0001.md) for actual part/pinout/capacity
- Binding architectural reference: [`docs/rom-flash-image-contract.md`](../../rom-flash-image-contract.md)
- This ticket **does not** authorise additional CPLD SPI-master RTL or relocate RTC/STM32 serial-flash responsibilities.

### Fixed logical ROM address map

Use `ROM_EN_N` + `MEM_ADDR_OUT` to address one raw contiguous array. The current MMU decodes a 64 KiB logical image:

- `0x0000–0x7FFF`: eight 4 KiB slot-index regions (slot 0 reserved; slots 1–7).
- Slot `N` entry ROM: `(N<<12)|(N<<8)`, 256 bytes in first 2 KiB of its window (IOSEL).
- Slot `N` expansion ROM: `(N<<12)|0x800`, 2048 bytes in second 2 KiB (IOSTB).
- Slot 6 Apple IIe `$C600–$C6FF` source must appear at physical `0x6600–0x66FF` when included; slot 7 entry at `0x7700` and expansion at `0x7800`.
- `0x8000–0xFFFF`: original 32 KiB system-ROM image; D000–FFFF system reads remain raw.

**Important:** DIP32 specifies the package only. Confirm part number, actual capacity/address lines, CE/OE/WE and programmer full-chip image size in `CPLD-HW-0001`; the 64 KiB decoded layout is not proof of a 64 KiB flash chip. A larger device still receives **one** deterministic full-chip image when required.

### Deliverables and constraints

1. A documented deterministic build/packing tool for one **headerless** physical programmer binary (suggested `apple_cpld_flash.bin`), with independent map/manifest/hash as optional sidecars. Never program `CPRM`/descriptor metadata at raw ROM offset 0.
2. The existing `*.romimg` format may remain as an intermediate, but the final result is one continuous byte-for-byte address-mapped `*.bin`; distinguish logical 64 KiB addressing from any full-chip padding/bank policy.
3. An emulator **raw-flash mode** which takes the exact raw programmer binary as its sole ROM contents, disabling `apple_rom_bin` fallback, slot migration and per-slot overlays in that mode. Every CPU ROM read must use the normal `ROM_EN_N`/physical `MEM_ADDR_OUT` path.
4. Regression tests comparing raw file bytes with both direct physical ROM reads and real MMU→memory bus CPU-visible fetches; verify slot boundaries, slot 6 legacy ROM, slot 7 firmware, per-slot C800 expansion, CFFF semantics and system ROM vectors. Avoid treating an independent copy of the RTL formula as a full integration test.
5. Rebuild deterministically twice, verify identical SHA-256; boot the emulator with the raw image and verify `PR#3`/`PR#0`, floppy paths and (when available) slot7 ProDOS/IOMEM. The owner has already confirmed `PR#3`/`PR#0` for the older reconstructed ROM; **repeat that test in raw mode**.
6. Document hardware programming/readback procedure and verify complete byte-for-byte readback against the very file tested in the emulator. If the chip is unavailable, mark this subtest **PENDING HARDWARE VERIFICATION**, not complete.

**Notes**

- **2026-11-21 — Owner Directive:** Hardware ROM must consist of **one image written to the DIP32 flash**. The emulator must reproduce that same image exactly to establish target-device confidence. This supersedes any assumption that per-slot development containers or RAM overlays are the final physical flash format.
- **2026-11-21 — Design Boundary:** The STM32 still acts as SPI master and owns RTC/serial flash. The DIP32 hardware ROM is a separate, directly addressed hardware resource. Do not conflate it with STM32-side SPI storage.
- **2026-11-21 — Baseline Test Evidence:** The owner reports the emulator works and `PR#3` / `PR#0` pass with the developer's slot-ROM remapping; this verifies the existing emulator mode, **not** a programmer-ready raw image.

**Proposed Change**

Adopt [the DIP32 flash image contract](../../rom-flash-image-contract.md), implement a deterministic raw-image composer and an opt-in raw-flash boot path, and integrate with the existing `rom_backing.c/.h` module. Keep `docs/rom-memory-map.md` authoritative for address interpretation and maintain the old baseline workflow only as a development option. Identify the exact DIP32 part in `CPLD-HW-0001` before fixing output device size or high address-line policy.

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

**Resolution Description**

Pending. Creating this ticket records the required deliverable and tests, not implementation or hardware verification.
