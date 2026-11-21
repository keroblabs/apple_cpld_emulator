# ROM and Expansion-ROM Memory Map (CPLD-ROM-0002)

> **Owner-approved pending image-size change (2026-11-21):** The 256 KiB / 0x40000 backing described below is the **tested historical rev-4 baseline**. [CPLD-ROM-0005](tasks/db/CPLD-ROM-0005.md) specifies the new **128 KiB SST39VF010-70** target: first 0x10000 bytes *unchanged*, upper 0x10000–0x1FFFF zero/reserved. Developer must revise this document to rev 5 after implementing/verifying the new export. Do not use old 256 KiB size for the target programmer.

**Status:** Specification for review, rev 4 (2026-11-21, `mmu-rom-analysis`)

The emulator assembles its ROM from compiled arrays. It can write those same
bytes as a single headerless 256 KiB DIP32 programmer image. The offsets below
apply to both consumers. The hardware ticket owns the physical wiring and
programmer readback.
[DIP32 flash-image contract](rom-flash-image-contract.md)

**Tickets:** [MMU decode](tasks/db/CPLD-ROM-0001.md), [ROM layout](tasks/db/CPLD-ROM-0002.md), [emulator backing](tasks/db/CPLD-ROM-0003.md)

## 1. Emulator ROM offsets

The emulator stores all ROM bytes in one 256 KiB array. Startup clears the
array to zero, copies `apple_rom_bin[]` to `0x08000`, and copies the six
compiled 4 KiB slot arrays to their windows. Apple image offset zero
corresponds to CPU address `$8000`. System ROM reads at `$D000–$FFFF` use
their CPU address as the array index, subject to the language-card switch.

| Array offsets | Size | Contents |
| --- | --- | --- |
| `0x00000–0x00FFF` | 4 KiB | Reserved slot 0 window. The released IOSTB window reads zero. |
| `0x01000–0x02FFF` | 8 KiB | Compiled slot 1 and slot 2 windows |
| `0x03000–0x03FFF` | 4 KiB | Empty card slot 3 window. The MMU selects Apple internal slot-3 ROM from `apple_rom_bin[]`. |
| `0x04000–0x07FFF` | 16 KiB | Compiled slot 4 through slot 7 windows |
| `0x08000–0x0BFFF` | 16 KiB | First half of the Apple image, outside the CPU ROM read windows |
| `0x0C000–0x0CFFF` | 4 KiB | Apple image bytes at their original offsets. `$C0xx` remains I/O and the internal Cx soft switches select `$C100–$CFFF`. |
| `0x0D000–0x0FFFF` | 12 KiB | Apple system ROM |
| `0x10000–0x3FFFF` | 192 KiB | Zero-filled reserved target capacity |

Each compiled slot array occupies `slot × 0x1000` and defaults to zero fill.
The [backing test](../emulator/onboard/verification/test_rom_backing.c) checks
the entire byte layout and both sides of the array bound.

## 2. Apple CPU address decode

| Slot | CPU IOSEL page | Array IOSEL page | Array IOSTB page for CPU `$C800–$CFFF` |
| --- | --- | --- | --- |
| 1 | `$C100–$C1FF` | `0x01000–0x010FF` | `0x01800–0x01FFF` |
| 2 | `$C200–$C2FF` | `0x02000–0x020FF` | `0x02800–0x02FFF` |
| 3 (empty card window) | `$C300–$C3FF` | `0x03000–0x030FF` | `0x03800–0x03FFF` |
| 4 | `$C400–$C4FF` | `0x04000–0x040FF` | `0x04800–0x04FFF` |
| 5 | `$C500–$C5FF` | `0x05000–0x050FF` | `0x05800–0x05FFF` |
| 6 | `$C600–$C6FF` | `0x06000–0x060FF` | `0x06800–0x06FFF` |
| 7 | `$C700–$C7FF` | `0x07000–0x070FF` | `0x07800–0x07FFF` |

- **Internal ROM.** When `rom_enabled` selects the original Apple ROM,
  `ROM_EN_N` is low and `MEM_ADDR_OUT` is the CPU address. This applies to
  system ROM and to the internal Cx bytes selected by `SW_SLOTXROM=1` or
  a selected slot 3 with `SW_SLOT3ROM=0`. The internal Cx selection via `$C007`
  still serves Apple `$C800` after `$CFFF`. Selecting external slot 3 reads
  the zero-filled card window.
- **Slot ROM.** A `$Cnxx` cycle latches `n` as the slot at the rising
  `PHI_0` edge. For a card ROM read, the page starts at
  `(slot_index << 12) | ADDR[7:0]`. `IOSEL_N` asserts low for external
  slots other than 6. The dedicated `C6XX_N` pin decodes slot 6, including
  when the internal Apple ROM owns that read.
- **Expansion ROM.** With card ROM selected, `IOSTB_N` asserts low and the
  index is `(slot_index << 12) | ADDR[11:0]`. On a card-ROM read, `$CFFF`
  delivers the selected slot's final byte at offset `$FFF`. Reads and
  writes to `$CFFF` both release the selector for the next cycle. The
  2 KiB range includes `$CFFF`; the range through `$CFFE` is 2047 bytes.
- **Mode selection.** With `MODE_SEL=1` the emulator reads the shared backing
  on `ROM_EN_N` or on a card ROM strobe (`IOSEL_N`, `IOSTB_N` or
  `C6XX_N`, all active low). Internal ROM has precedence at `$C6xx`
  when both `ROM_EN_N` and `C6XX_N` assert. With `MODE_SEL=0`, the MMU
  additionally asserts `ROM_EN_N` for external IOSEL/IOSTB reads. For an
  external `$C6xx` read, the dedicated decode remains the bus selection.
  `ROM_EN_N` stays high on writes in both modes.

The default MMU exposes address bits `[16:12]`. The emulator combines them
with the CPU offset, using bits `[7:0]` for a card IOSEL read and bits
`[11:0]` for RAM, internal ROM and IOSTB. `FULL_MMU_ADDR` supplies all 17
bits directly. Both builds are probed in the [MMU test](../rtl_MMU/verification/tb_mmu_addr.cpp)
and the [combined ROM bus test](../emulator/onboard/verification/test_rom_bus.cpp).
Physical bus arbitration and DIP32 ROM timing remain in [CPLD-HW-0001](tasks/db/CPLD-HW-0001.md).
A board using the 5-pin MMU and this raw slot layout needs the same IOSEL
low-address selection on its backing path.

## 3. Compiled slot ROM inputs

Build each populated slot as an independent 4096-byte ROM and include its
bytes as a C array. Link the IOSEL code for CPU `$Cn00–$CnFF` and the IOSTB
code for CPU `$C800–$CFFF`.

| File offset | Size | Contents |
| --- | --- | --- |
| `0x000–0x0FF` | 256 bytes | IOSEL page at the start of the first 2 KiB |
| `0x100–0x7FF` | 1792 bytes | Padding in the first 2 KiB |
| `0x800–0xFFF` | 2048 bytes | IOSTB expansion in the upper 2 KiB |

The six inputs are `slot1_rom_bin[]`, `slot2_rom_bin[]`, `slot4_rom_bin[]`,
`slot5_rom_bin[]`, `slot6_rom_bin[]` and `slot7_rom_bin[]`. The zero-filled
defaults are protected by `#ifndef SLOT1_ROM`, `#ifndef SLOT2_ROM`,
`#ifndef SLOT4_ROM`, `#ifndef SLOT5_ROM`, `#ifndef SLOT6_ROM` and
`#ifndef SLOT7_ROM`, respectively. Slots 0 and 3 receive only the array's
initial zero fill. [Default slot arrays](../emulator/onboard/slot_roms.h)

To supply slot 1, define `SLOT1_ROM` and place the definition of
`slot1_rom_bin[]` above the defaults in `slot_roms.h`, or put the definition
in a header and include it there. For example, with a `slot1_rom.h` containing
the array definition, add these lines above the fallback blocks.

```c
#define SLOT1_ROM
#include "slot1_rom.h"
```

Rebuild the emulator after changing any slot input. The ROM backing checks
each array's size at compile time and copies all 4096 bytes to its slot window.

For example, a slot 7 build places its entry at file offset `0x000` (array
index `0x07000`), its ProDOS metadata at offsets `0x0FC–0x0FF` (CPU
`$C7FC–$C7FF`), and its expansion at offset `0x800` (array index
`0x07800`). The firmware ticket owns the metadata byte values and entry
points. [CPLD-FW-0001](tasks/db/CPLD-FW-0001.md)

## 4. Export the target ROM image

```bash
build/apple_cpld -w /path/to/flash-image.bin
```

`-w IMAGEFILE` is the sole command-line option. It writes the populated
backing as exactly 262144 bytes in physical address order before the CPU
runs. On success the emulator runs from that backing. With no arguments the
emulator starts from the same arrays and creates no ROM file.

The output is the expected 256 KiB ROM contents for the real target. Device
wiring and full-chip readback are verified separately.
[DIP32 flash-image contract](rom-flash-image-contract.md)
