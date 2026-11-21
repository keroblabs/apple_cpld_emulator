# DIP32 Flash Image Contract

**Status:** Owner release requirement, reconciled with ROM map rev 3 on 2026-11-21.

**Tickets:** [Image composition and parity](tasks/db/CPLD-ROM-0004.md), [emulator loader](tasks/db/CPLD-ROM-0003.md), [hardware ROM](tasks/db/CPLD-HW-0001.md)

## 1. Release file

Share one contiguous, headerless binary between the emulator and DIP32
programmer. Byte `A` in the file provides the byte at physical ROM address
`A` under the documented wiring. Record the input manifest, section map,
length and SHA-256 separately. Release evidence comes from booting this exact
file directly and comparing it with the chip's complete programmer readback.

The development workflow can initialize the shared ROM array with the Apple
image and then copy independent 4 KiB slot files. For release verification the
supplied flat image provides all ROM bytes from startup. Its contents stay
fixed across software resets and slot-load requests.

`build/apple_cpld --rom IMAGE.bin` currently accepts exactly 256 KiB, the
capacity of the emulator array. Fixture tests exercise this startup path.
Confirm the hardware part and its required file length for the actual release.
[Whole-image startup](rom-memory-map.md#4-whole-image-emulator-startup)

## 2. Physical byte offsets

The current MMU selects ROM within the lowest 64 KiB of the 256 KiB emulator
backing. The same translated offsets apply to the flat file. [ROM memory map](rom-memory-map.md)

| Physical offsets | Contents |
| --- | --- |
| `0x00000–0x00FFF` | Reserved slot 0. Read the stored fill after expansion release. |
| `0x01000–0x07FFF` | Slots 1–7. Slot `N` starts at `N × 0x1000`. |
| `0x08000–0x0FFFF` | Original 32 KiB Apple image, including internal Cx and `$D000–$FFFF` system ROM |
| `0x10000–0x3FFFF` | Reserved emulator capacity. Device size and high-address wiring need hardware confirmation. |

Copy each independently built 4096-byte slot binary verbatim into its window.
The first 256 bytes are IOSEL. The remaining bytes through `+0x7FF` are padding.
IOSTB occupies `+0x800–+0xFFF`. The final IOSTB byte is returned at `$CFFF`
before the selector releases on the following cycle.

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
Record the address-line wiring, high-bank policy, CE/OE/WE control, erased byte
value and programmer settings. Decide the fill for unused and inaccessible
bytes in the file. The runtime array size and the size of the currently decoded
map are emulator properties. The device specification establishes the
programmer input size. [CPLD-HW-0001](tasks/db/CPLD-HW-0001.md)

The STM32 remains responsible for its SPI bus, serial flash and RTC. The
DIP32 ROM serves the CPU via the CPLD memory bus.

## 4. Build and verify

1. Compose a raw file from the unchanged Apple ROM, each available 4096-byte slot file and the agreed fill. Validate the input sizes and destinations. Record the fill and any explicitly unpopulated slot.
2. Rebuild the file independently and compare size, bytes and SHA-256. Confirm its length and fill against the verified device requirements.
3. Start the emulator from the exact file. Compare physical bytes and MMU-driven reads for all slot entries, all IOSTB windows including CFFF, Apple internal ROM and reset vectors. Repeat Apple startup, Applesoft `PR#3`/`PR#0` and the floppy bootstrap. Run the slot-7 ProDOS boot and byte-exact IOMEM transfer when its firmware and disk service are available.
4. Program the DIP32 device using that same file and compare the entire readback. Record the device, programmer parameters, binary hash, source commit and results.

Hardware readiness requires all four stages. The owner's `PR#3`/`PR#0`
report covers the earlier development configuration. Actual programmer-file
composition, the test with that file and physical readback remain in
[CPLD-ROM-0004](tasks/db/CPLD-ROM-0004.md).
