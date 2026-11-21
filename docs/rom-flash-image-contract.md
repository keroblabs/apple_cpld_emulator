# SST39VF010-70 Flash Image Contract

**Status:** Owner release requirement, aligned with ROM map rev 5 on 2026-11-21. Physical verification is pending.

**Tickets:** [Image export and parity](tasks/db/CPLD-ROM-0004.md), [128 KiB retarget](tasks/db/CPLD-ROM-0005.md), [hardware ROM](tasks/db/CPLD-HW-0001.md)

## 1. Release file

The emulator assembles its startup backing from `apple_rom_bin[]` and
the six compiled 4096-byte slot arrays. With `-w IMAGEFILE` it writes that
same backing as one contiguous, headerless 131072-byte programmer file.
Byte `A` in the file is physical ROM address `A` under the documented wiring.
The CPU runs from the backing that was written. With no arguments the
emulator runs from the same compiled arrays and creates no image.

Record the source revision, populated slot inputs, section map, byte length
and SHA-256 separately. Verify the complete programmer readback against the
emulator's output. [Export the target ROM image](rom-memory-map.md#4-export-the-target-rom-image)

## 2. Physical byte offsets

The current MMU selects ROM within the lower 64 KiB of the 128 KiB emulator
backing. The same translated offsets apply to the flat file. [ROM memory map](rom-memory-map.md)

| Physical offsets | Contents |
| --- | --- |
| `0x00000–0x00FFF` | Slot 0. Zero fill after expansion release. |
| `0x01000–0x02FFF` | Compiled slots 1 and 2 |
| `0x03000–0x03FFF` | Zero-filled card slot 3. Apple internal slot-3 ROM comes from the Apple image. |
| `0x04000–0x07FFF` | Compiled slots 4–7 |
| `0x08000–0x0FFFF` | Original 32 KiB Apple image, including internal Cx and `$D000–$FFFF` system ROM |
| `0x10000–0x1FFFF` | Reserved, zero-filled upper 64 KiB, inaccessible from the CPU ROM decode |

The image copies the compiled 4096-byte array for slot `N` at `N × 0x1000`.
The first 256 bytes are IOSEL. The remaining bytes through `+0x7FF` are padding.
IOSTB occupies `+0x800–+0xFFF`. The final IOSTB byte is returned at `$CFFF`
before the selector releases on the following cycle. Default slot arrays are zero.

- Slot 7 entry is at `0x07000–0x070FF`. Its expansion is at `0x07800–0x07FFF`.
- The internal Apple `$C600–$C6FF` bytes stay at `0x0C600–0x0C6FF`. When a separate Disk II card ROM is included, its entry begins at `0x06000`. Physical selection and `C6XX_N` bus ownership require hardware verification.
- A board exposing only `MEM_ADDR_OUT[16:12]` must select the IOSEL low address so physical A11–A8 are zero for a card entry. For IOSTB and Apple ROM it uses the normal lower 12 CPU address bits. The emulator implements that selection in both its 5-bit and 17-bit address builds.

The production memory controller uses the sampled MMU address. Emulator mode
selects a read through `ROM_EN_N` or the card ROM strobes `IOSEL_N`,
`IOSTB_N` and `C6XX_N`. Physical arbitration must grant each cycle to one
driver, including when an optional external controller owns slot 6.

## 3. Hardware inputs to confirm

The SST39VF010-70 is 128K × 8, with A0–A16 and a 70 ns address access
rating. Strap flash A16 to VSS on the target. RAM bank bit 16 is a separate
signal and the flash has no runtime bank switching. The programmer must
address the entire chip and write the zero-filled upper bank too.

Use a regulated 3.3 V supply and verify the voltage domains on all address,
control and data lines. Hold WE# at flash VDD in the target. Confirm the
full part suffix and use the matching PLCC32 or TSOP32 footprint. A DIP32
socket requires a pin-mapped adapter or an approved footprint change.
Verify CE#/OE# arbitration, physical timing and the complete readback.
[CPLD-HW-0001](tasks/db/CPLD-HW-0001.md#sst39vf010-70-reference-connections)

The STM32 remains responsible for its SPI bus, serial flash and RTC. The
parallel flash serves the CPU via the CPLD memory bus.

## 4. Build and verify

1. Build the emulator with the desired 4096-byte arrays and their `SLOTn_ROM` definitions. Record the input revisions and unpopulated slots.
2. Start it with `-w IMAGEFILE`. Compare the exported 128 KiB image with the compiled inputs and fill at the documented offsets. Repeat the build and compare bytes and SHA-256.
3. Compare MMU-driven reads from that startup backing with the emitted bytes. Probe IOSEL, IOSTB including CFFF, the reserved regions, internal Apple ROM and reset vectors. Verify Apple startup, Applesoft `PR#3`/`PR#0` and the floppy bootstrap. Run slot-7 ProDOS and a byte-exact IOMEM transfer when its firmware and disk service are available.
4. Program the SST39VF010-70 using that same file and compare the entire 131072-byte readback. Record the device, programmer parameters, binary hash, source commit and results.

Hardware readiness requires all four stages. Physical programming and
complete chip readback are tracked in [CPLD-ROM-0004](tasks/db/CPLD-ROM-0004.md).
