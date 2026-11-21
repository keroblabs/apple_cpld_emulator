# DIP32 Flash Image Contract

> **Owner-approved pending device change (2026-11-21):** [CPLD-ROM-0005](tasks/db/CPLD-ROM-0005.md) changes the physical target from 256 KiB to **128 KiB SST39VF010-70** (131072 bytes). The older 262144-byte image statements below are historical until the developer updates this contract. The selected VF010 is a **low-voltage PLCC32/TSOP32** part in the manufacturer datasheet, not a verified drop-in DIP32. No physical wiring or programming approval is implied.

**Status:** Owner release requirement, aligned with ROM map rev 4 on 2026-11-21.

**Tickets:** [Image export and parity](tasks/db/CPLD-ROM-0004.md), [emulator backing](tasks/db/CPLD-ROM-0003.md), [hardware ROM](tasks/db/CPLD-HW-0001.md)

## 1. Release file

The emulator assembles its startup backing from `apple_rom_bin[]` and
the six compiled 4096-byte slot arrays. With `-w IMAGEFILE` it writes that
same backing as the one contiguous, headerless 262144-byte programmer file.
Byte `A` in the file is physical ROM address `A` under the documented wiring.
The CPU runs from the backing that was written. With no arguments the
emulator runs from the same compiled arrays and creates no image.

Record the source revision, populated slot inputs, section map, byte length
and SHA-256 separately. Verify the complete programmer readback against the
emulator's output. [Export the target ROM image](rom-memory-map.md#4-export-the-target-rom-image)

## 2. Physical byte offsets

The current MMU selects ROM within the lowest 64 KiB of the 256 KiB emulator
backing. The same translated offsets apply to the flat file. [ROM memory map](rom-memory-map.md)

| Physical offsets | Contents |
| --- | --- |
| `0x00000–0x00FFF` | Slot 0. Zero fill after expansion release. |
| `0x01000–0x02FFF` | Compiled slots 1 and 2 |
| `0x03000–0x03FFF` | Zero-filled card slot 3. Apple internal slot-3 ROM comes from the Apple image. |
| `0x04000–0x07FFF` | Compiled slots 4–7 |
| `0x08000–0x0FFFF` | Original 32 KiB Apple image, including internal Cx and `$D000–$FFFF` system ROM |
| `0x10000–0x3FFFF` | Zero-filled high target addresses. Verify the hardware address wiring. |

The image copies the compiled 4096-byte array for slot `N` at `N × 0x1000`.
The first 256 bytes are IOSEL. The remaining bytes through `+0x7FF` are padding.
IOSTB occupies `+0x800–+0xFFF`. The final IOSTB byte is returned at `$CFFF`
before the selector releases on the following cycle. Default slot arrays are zero.

- Slot 7 entry is at `0x07000–0x070FF`. Its expansion is at `0x07800–0x07FFF`.
- The internal Apple `$C600–$C6FF` bytes stay at `0x0C600–0x0C6FF`. When a separate Disk II card ROM is included, its entry begins at `0x06000`. Physical selection and `C6XX_N` bus ownership require hardware verification.
- A board exposing only `MEM_ADDR_OUT[16:12]` must select the IOSEL low address so physical A10–A8 are zero for a card entry. For IOSTB and Apple ROM it uses the normal lower 12 CPU address bits. The emulator implements that selection in both its 5-bit and 17-bit address builds.

The production memory controller uses the sampled MMU address. Emulator mode
selects a read through `ROM_EN_N` or the card ROM strobes `IOSEL_N`,
`IOSTB_N` and `C6XX_N`. The hardware ticket must establish which select
signals actually enable the DIP32 data bus and how an optional external
controller owns the slot-6 cycle.

## 3. Hardware inputs to confirm

Identify the DIP32 part number, byte capacity, voltage and electrical timing.
Record the address-line wiring, high-bank selection, CE/OE/WE control, erased
byte value and programmer settings. Confirm that the device and programmer
can consume and read back the full 256 KiB image, including its zero-filled
areas. [CPLD-HW-0001](tasks/db/CPLD-HW-0001.md)

The STM32 remains responsible for its SPI bus, serial flash and RTC. The
DIP32 ROM serves the CPU via the CPLD memory bus.

## 4. Build and verify

1. Build the emulator with the desired 4096-byte arrays and their `SLOTn_ROM` definitions. Record the input revisions and unpopulated slots.
2. Start it with `-w IMAGEFILE`. Compare the exported 256 KiB image with the compiled inputs and fill at the documented offsets. Repeat the build and compare bytes and SHA-256.
3. Compare MMU-driven reads from that startup backing with the emitted bytes. Probe IOSEL, IOSTB including CFFF, the reserved regions, internal Apple ROM and reset vectors. Verify Apple startup, Applesoft `PR#3`/`PR#0` and the floppy bootstrap. Run slot-7 ProDOS and a byte-exact IOMEM transfer when its firmware and disk service are available.
4. Program the DIP32 device using that same file and compare the entire readback. Record the device, programmer parameters, binary hash, source commit and results.

Hardware readiness requires all four stages. Physical programming and
complete chip readback are tracked in [CPLD-ROM-0004](tasks/db/CPLD-ROM-0004.md).
