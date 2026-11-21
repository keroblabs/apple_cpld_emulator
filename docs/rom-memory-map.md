# ROM and Expansion-ROM Memory Map (CPLD-ROM-0002)

**Status:** Specification for review, rev 3 (2026-11-21, `mmu-rom-analysis`)

**Tickets:** [MMU decode](tasks/db/CPLD-ROM-0001.md), [ROM layout](tasks/db/CPLD-ROM-0002.md), [emulator backing](tasks/db/CPLD-ROM-0003.md)

## 1. Emulator ROM offsets

The emulator stores all ROM bytes in one 256 KiB array. Offsets below are byte
indices into that array. At startup the board clears the array to zero and
copies the complete original 32 KiB Apple ROM to `0x08000`. Its offset zero
corresponds to Apple address `$8000`. System ROM reads at `$D000–$FFFF` use
their CPU address as the array index, subject to the language-card switch.

| Array offsets | Size | Contents |
| --- | --- | --- |
| `0x00000–0x00FFF` | 4 KiB | Reserved slot 0 window. The released IOSTB window reads zero. |
| `0x01000–0x07FFF` | 28 KiB | Seven separate 4 KiB slot windows |
| `0x08000–0x0BFFF` | 16 KiB | First half of the Apple image, outside the CPU ROM read windows |
| `0x0C000–0x0CFFF` | 4 KiB | Apple image bytes at their original offsets. `$C0xx` remains I/O and the internal Cx soft switches select `$C100–$CFFF`. |
| `0x0D000–0x0FFFF` | 12 KiB | Apple system ROM |
| `0x10000–0x3FFFF` | 192 KiB | Reserved capacity |

Each slot binary is copied at `slot × 0x1000`. ROM reads from unpopulated slots
return zero. The standalone [backing test](../emulator/onboard/verification/test_rom_backing.c)
checks byte-for-byte copies, slot isolation and both sides of the array bound.

## 2. Apple CPU address decode

| Slot | CPU IOSEL page | Array IOSEL page | Array IOSTB page for CPU `$C800–$CFFF` |
| --- | --- | --- | --- |
| 1 | `$C100–$C1FF` | `0x01000–0x010FF` | `0x01800–0x01FFF` |
| 2 | `$C200–$C2FF` | `0x02000–0x020FF` | `0x02800–0x02FFF` |
| 3 | `$C300–$C3FF` | `0x03000–0x030FF` | `0x03800–0x03FFF` |
| 4 | `$C400–$C4FF` | `0x04000–0x040FF` | `0x04800–0x04FFF` |
| 5 | `$C500–$C5FF` | `0x05000–0x050FF` | `0x05800–0x05FFF` |
| 6 | `$C600–$C6FF` | `0x06000–0x060FF` | `0x06800–0x06FFF` |
| 7 | `$C700–$C7FF` | `0x07000–0x070FF` | `0x07800–0x07FFF` |

- **Internal ROM.** When `rom_enabled` selects the original Apple ROM,
  `ROM_EN_N` is low and `MEM_ADDR_OUT` is the CPU address. This applies to
  system ROM and to the internal Cx bytes selected by `SW_SLOTXROM=1` or
  a selected slot 3 with `SW_SLOT3ROM=0`. The internal Cx selection via `$C007` still
  serves Apple `$C800` after `$CFFF`.
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
Physical bus arbitration and STM32 flash timing remain in [CPLD-HW-0001](tasks/db/CPLD-HW-0001.md).
A board using the 5-pin MMU and this raw slot layout needs the same IOSEL
low-address selection on its backing path.

## 3. Raw slot binary and startup API

Build each slot as its own 4096-byte binary. Link the IOSEL code for CPU
`$Cn00–$CnFF` and the IOSTB code for CPU `$C800–$CFFF`. The build places the
bytes at these file offsets.

| File offset | Size | Contents |
| --- | --- | --- |
| `0x000–0x0FF` | 256 bytes | IOSEL page at the start of the first 2 KiB |
| `0x100–0x7FF` | 1792 bytes | Padding in the first 2 KiB |
| `0x800–0xFFF` | 2048 bytes | IOSTB expansion in the upper 2 KiB |

Call `memory_controller_load_slot_rom(slot, bytes, length)` after board or
memory-controller initialization and before the CPU runs. The caller supplies the
slot number and exact file length. The loader copies the entire file verbatim
and returns 0. It returns -1 for an invalid slot, null pointer or length other
than 4096 bytes. A successful repeat load replaces the whole slot window.
Initialization clears all slot windows. Default startup loads the Apple ROM
alone.

For example, a slot 7 build places its entry at file offset `0x000` (array
index `0x07000`), its ProDOS metadata at offsets `0x0FC–0x0FF` (CPU
`$C7FC–$C7FF`), and its expansion at offset `0x800` (array index
`0x07800`). The firmware ticket owns the metadata byte values and entry
points. [CPLD-FW-0001](tasks/db/CPLD-FW-0001.md)
