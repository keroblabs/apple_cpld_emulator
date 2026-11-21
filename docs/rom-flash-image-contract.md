# DIP32 Flash Image Contract — One Programmer-Ready ROM

**Status:** Owner directive, accepted as the target deliverable (2026-11-21); implementation and hardware validation pending.  
**Canonical implementation ticket:** [CPLD-ROM-0004](tasks/db/CPLD-ROM-0004.md).  
**Address-map source:** [ROM memory map](rom-memory-map.md).  
**Related:** [CPLD-ROM-0001](tasks/db/CPLD-ROM-0001.md), [CPLD-ROM-0002](tasks/db/CPLD-ROM-0002.md), [CPLD-ROM-0003](tasks/db/CPLD-ROM-0003.md), [CPLD-HW-0001](tasks/db/CPLD-HW-0001.md).

## 1. Non-negotiable release rule

**One binary, two consumers.** The final firmware/ROM delivery is **a single, contiguous, raw binary** whose byte at file offset `A` is the byte the DIP32 flash returns when the ROM address bus presents `A` (subject only to the *documented* address-bus/chip-capacity wiring). The *same, unmodified binary* must be loaded as the emulator's ROM backing image and used in emulator regression tests **before** it is programmed into the real flash.

A successful emulator boot using reconstructed arrays, separately loaded entry/expansion images, patches, hidden remapping or runtime overlays is **not** sufficient to validate the physical flash image.

**Do not program the existing `*.romimg` container directly.** Its `CPRM` header, descriptor table and payload offsets are a *development packaging convention*, not the flash's physical address map. A packer may use it as input, but must emit one **headerless, programmer-ready** `*.bin` (example name: `apple_cpld_flash.bin`). Human-readable map/manifest/hash files may be generated as sidecars, never as prefixes or embedded metadata within the raw ROM address range.

## 2. Authoritative 64 KiB decoded ROM map

This table is the logical byte-addressed space selected by `ROM_EN_N` and `MEM_ADDR_OUT` in the present RTL. It describes the lower **0x10000-byte decoded map**; it does **not** establish the yet-unconfirmed capacity or pinout of the chosen DIP32 flash device.

| Raw image offset | Length | Content |
| --- | ---: | --- |
| `0x0000–0x0FFF` | 4 KiB | Slot-index 0 window; defined deterministic padding/fallback |
| `0x1000–0x1FFF` | 4 KiB | Slot 1, IOSEL first 2 KiB / IOSTB second 2 KiB |
| `0x2000–0x2FFF` | 4 KiB | Slot 2, same geometry |
| `0x3000–0x3FFF` | 4 KiB | Slot 3, same geometry |
| `0x4000–0x4FFF` | 4 KiB | Slot 4, same geometry |
| `0x5000–0x5FFF` | 4 KiB | Slot 5, same geometry |
| `0x6000–0x6FFF` | 4 KiB | Slot 6, same geometry |
| `0x7000–0x7FFF` | 4 KiB | Slot 7, same geometry |
| `0x8000–0xFFFF` | 32 KiB | Original system ROM image, including the D000–FFFF system-ROM bytes |

For slot `N` in 1..7:

- **IOSEL:** Apple `$Cn00–$CnFF` is at raw file offsets `(N << 12) | (N << 8)` through `+0xFF`. This is **256 bytes** within the first 2 KiB of the slot's 4 KiB window; any other bytes in that first half have an explicit padding/default policy, not unspecified/uninitialised contents.
- **IOSTB:** Apple's selected `$C800–$CFFF` maps into raw offsets `(N << 12) | 0x800` through `(N << 12) | 0xFFF`, **2 KiB** per slot.
- **Slot 6:** Preserve the Apple IIe's original `$C600–$C6FF` firmware at physical offsets `0x6600–0x66FF` when retaining the integrated baseline drive firmware. Keep optional external-controller `C6XX_N` compatibility and bus-ownership policy separately reviewed.
- **Slot 7:** `$C700–$C7FF` at `0x7700–0x77FF`; expansion `$C800–$CFFF` at `0x7800–0x7FFF`. The ProDOS/IOMEM firmware must be packed directly into these positions.
- **System ROM:** `$D000–$FFFF` uses raw addresses `0xD000–0xFFFF`, unmodified by slot selection. The `0x8000–0xCFFF` source-image portion may also be populated and must be deterministic.

The selected-slot/CFFF timing question in [CPLD-ROM-0001](tasks/db/CPLD-ROM-0001.md) must be resolved from actual clock-phase evidence before claiming an exact `$CFFF` data-byte mapping.

## 3. Physical DIP32 target constraints — confirm before locking output size

**DIP32 describes a package, not a unique flash capacity, address bus, programming algorithm or voltage.** [CPLD-HW-0001](tasks/db/CPLD-HW-0001.md) must identify the actual part number and confirm:

- Flash capacity in bytes, number of address lines, which lines are driven by `MEM_ADDR_OUT` (including any high address bits), how `CE/OE/WE` are controlled using `ROM_EN_N`/read-write direction, and flash-to-CPU data-bus timing.
- Whether the programmer needs a **complete device-capacity image**. If the device is larger than the 64 KiB decoded address space, define a deterministic high-bank/padding policy and produce one **full-chip binary** when required. Do not silently repeat/mirror the 64 KiB image or assume A16 is tied a particular way.
- Flash erased-state/padding value and any unused/blocked address ranges. Physical bytes must be completely specified, not inferred from the emulator's historical `0xA0` idle-bus convention.

The logical 64 KiB map above is fixed for the current MMU design, **not** a claim that the selected DIP32 chip has exactly 64 KiB capacity.

## 4. Reproducible build and exact emulator execution

1. Define authoritative inputs: original system ROM bytes, slot 1–7 IOSEL entry pages, individual slot IOSTB images, and a deterministic fill rule for every unallocated address. Maintain a versioned manifest specifying source, destination offset and size; reject overlapping inputs, absent mandatory regions or out-of-map content.
2. Emit one **raw** programmer image, not a container. For the currently decoded 16-bit physical space the logical image is 65536 bytes; the final programmer file size must match the chosen hardware device requirements above. Include an independently calculated `SHA-256` and a separate human-readable section map.
3. Add an explicit **raw-flash mode** to emulator startup: load these bytes as the sole physical ROM array, with no `apple_rom_bin` bootstrap copy, no per-slot baseline migration and no late overlays in that mode. CPU ROM reads continue to use **only** `ROM_EN_N` and `MEM_ADDR_OUT` as at present. Validate exact file length/address range.
4. Keep the existing assembled/baseline ROM workflow if desired for development, but do not mistake tests using it for tests using the final flash image.
5. Run exact-byte probes at slot 1–7 entry addresses, each slot's C800/CFFF expansion path (once semantics are locked), `0x6600`, `0x7700–0x77FF`, `0x7800–0x7FFF`, system reset vectors and `0xD000–0xFFFF`. Exercise representative normal CPU reads through the integrated MMU/memory controller.
6. Boot the emulator in raw-flash mode and repeat the owner-visible regression (Apple II boot, `PR#3` and `PR#0`, and floppy paths). When available, verify that the slot 7 ProDOS firmware loads and transfers an IOMEM 512-byte block **from this exact image**.
7. After programming the chip, **read it back** using the programmer and compare the full device image byte-for-byte, including SHA-256, against the file tested in the emulator. Record part number, programmer parameters, commit, binary hash and exact tests.

## 5. Acceptance gate

Do **not** label the ROM deliverable 'hardware ready' until all are true:

- [ ] One deterministic raw `*.bin` is built with no container header and no unspecified bytes.
- [ ] Hardware part/pinout/capacity have been confirmed and the binary is the exact programmer input (including high-bank policy if applicable).
- [ ] The emulator boots/operates **using that binary directly** and logs its hash; it does not synthesise or patch ROM bytes after loading.
- [ ] Address probes and representative CPU fetches match the documented MMU map, including the migrated `$C6xx` bytes and slot 7 contents.
- [ ] The same image is programmed and the chip readback matches byte-for-byte; when physical hardware is not yet present, this step remains expressly **PENDING HARDWARE VERIFICATION**, not silently passed.

The owner's successful `PR#3` / `PR#0` verification on the current reconstructed-ROM emulator is preserved as existing baseline regression evidence, **not** evidence of the future single-image raw-flash mode or hardware readback.
