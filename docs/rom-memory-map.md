# ROM and Expansion-ROM Memory Map (CPLD-ROM-0002)

**Status:** Specification for review (2026-11-20, branch `mmu-rom-analysis`).
**Methodology:** [Task and Bug Tracking Methodology](tasks/bug-tracking-methodology.md).
**Owning ticket:** [`CPLD-ROM-0002`](tasks/db/CPLD-ROM-0002.md).
**Depends on:** [`CPLD-ROM-0001`](tasks/db/CPLD-ROM-0001.md) (verified address behaviour),
[`CPLD-HW-0001`](tasks/db/CPLD-HW-0001.md) (physical ROM-read topology).

This document defines a durable, collision-free ROM image layout for per-slot
firmware and the shared expansion-ROM window, and maps the **Apple-visible
memory windows** to **physical flash/image offsets**. It separates two distinct
address spaces that must not be conflated:

1. **Apple CPU address space** — the `$Cxxx`/`$Dxxx`–`$Fxxx` windows the 65C816
   presents on `ADDR[15:0]`.
2. **Physical ROM-image space** — the byte offsets of the packed image that the
   emulator (and later the STM32-managed serial flash) stores and serves.

## 1. Grounding fact from CPLD-ROM-0001

The MMU address behaviour was verified by
[`rtl_MMU/verification/tb_mmu_addr.cpp`](../rtl_MMU/verification/tb_mmu_addr.cpp).
The single most important result for this layout:

> With `MODE_SEL = 1` (the standing emulator configuration, forced in
> `cpld/rtl_cpu_bus.cpp`), `onboard_fw_n` is always high, so the MMU's
> slot-index translation branch is **never** taken. Firmware ROM reads use the
> `rom_enabled` branch and **`MEM_ADDR_OUT` equals the raw CPU address**
> (bit 16 = 0). The stateful slot selector does **not** translate ROM
> addresses in the emulator.

Consequences for this map:

- Per-slot entry ROM is addressed by its **CPU `$Cn00`** window directly, not by
  a slot-translated offset. Physical image offset = CPU address for the ROM
  region.
- The shared expansion window `$C800–$CFFF` is, in the emulator's raw-address
  model, a **single** physical 2 KiB region. Only one slot's expansion image can
  occupy those CPU bytes at a time; the design therefore **reserves the
  expansion window for slot 7** (ProDOS). True hardware multi-card expansion
  (which relies on `MODE_SEL = 0` slot translation or external card decode) is a
  hardware-topology concern tracked in `CPLD-HW-0001`, not a change to the
  Apple-visible ROM contents here.

## 2. Apple CPU address windows

| Apple window | Size | Role | Selected by | ROM_EN_N | MEM_ADDR_OUT (emulator) |
| --- | --- | --- | --- | --- | --- |
| `$C000–$C0FF` | 256 B | Soft switches / I/O (incl. C0Fx mailbox) | `C0XX_N` | 1 (not ROM) | — |
| `$C100–$C1FF` | 256 B | Slot 1 entry ROM | IOSEL, `SW_SLOTXROM` | 0 | `$C100–$C1FF` |
| `$C200–$C2FF` | 256 B | Slot 2 entry ROM | IOSEL, `SW_SLOTXROM` | 0 | `$C200–$C2FF` |
| `$C300–$C3FF` | 256 B | Slot 3 entry ROM | IOSEL, `SW_SLOTXROM`/`SW_SLOT3ROM` | 0 | `$C300–$C3FF` |
| `$C400–$C4FF` | 256 B | Slot 4 entry ROM | IOSEL, `SW_SLOTXROM` | 0 | `$C400–$C4FF` |
| `$C500–$C5FF` | 256 B | Slot 5 entry ROM | IOSEL, `SW_SLOTXROM` | 0 | `$C500–$C5FF` |
| `$C600–$C6FF` | 256 B | Slot 6 entry ROM (disk II / ext. controller) | IOSEL, `SW_SLOTXROM`; `C6XX_N` for external | 0 (onboard) | `$C600–$C6FF` |
| `$C700–$C7FF` | 256 B | **Slot 7 entry ROM (ProDOS boot/driver)** | IOSEL, `SW_SLOTXROM` | 0 | `$C700–$C7FF` |
| `$C800–$CFFE` | 2046 B | Shared expansion ROM window | IOSTB, after a slot IOSEL; reset by `$CFFF` | 0 | `$C800–$CFFE` |
| `$CFFF`       | 1 B | Expansion-window release strobe | IOSTB | 0 | `$CFFF` |
| `$D000–$DFFF` | 4 KiB | System ROM (bank-switched / LC) | `D_FXXX_INT` | 0 | `$D000–$DFFF` |
| `$E000–$FFFF` | 8 KiB | System ROM (monitor/BASIC) | `D_FXXX_INT` | 0 | `$E000–$FFFF` |

Notes:
- `C6XX_N` is preserved as a dedicated decode for an **optional external slot-6
  controller**; the onboard slot-6 entry ROM and the external decode must not
  both drive the bus (see `CPLD-HW-0001`).
- ProDOS firmware metadata lives in the **last four bytes of the slot entry
  ROM**, `$CnFC–$CnFF`. For slot 7 that is `$C7FC–$C7FF`; these must coexist
  with the boot and driver entry points packed into `$C700–$C7FB`
  (see `CPLD-FW-0001`).

## 3. Emulator logical ROM space (CPU-readable path)

The emulator exposes a **64 KiB logical ROM space** indexed by CPU address
(`MEM_ADDR_OUT & 0xFFFF`, bit 16 = 0 for firmware ROM). Only the upper half is
populated for firmware; the lower half is reserved/erased.

| Logical offset | CPU window | Contents |
| --- | --- | --- |
| `$0000–$7FFF` | — | Reserved / erased (`0xA0` fill); not a ROM window in normal operation |
| `$8000–$C0FF` | `$8000–$C0FF` | System ROM image body (from `apple_rom_bin`) |
| `$C100–$C6FF` | slots 1–6 entry | System/slot entry ROM (from image; per-slot overlay optional) |
| `$C700–$C7FF` | slot 7 entry | **Slot 7 ProDOS entry ROM overlay** |
| `$C800–$CFFF` | expansion | **Slot 7 expansion ROM overlay** (reserved owner) |
| `$D000–$FFFF` | system ROM | Monitor / BASIC / LC ROM (from image) |

The existing emulator loads `apple_rom_bin` (32 KiB) at logical offset
`$8000`, so the system ROM already covers `$8000–$FFFF`. Slot-7 overlays are
written **on top of** the `$C700–$C7FF` and `$C800–$CFFF` regions after the
system image load. This guarantees:

- No two active ROM windows alias the same unintended physical byte: each CPU
  address maps to exactly one logical offset, and overlays replace (not alias)
  the system image bytes they occupy.
- The Apple-visible contents of the unmodified slots (1–6) and the system ROM
  (`$D000–$FFFF`) are unchanged when no overlay is configured.

## 4. Physical flash layout (STM32-managed serial flash)

Physical serial-flash storage is **separate** from the CPU-readable path. The
STM32 owns the flash; how a physical ROM read is serviced in real time (SRAM
shadow, parallel ROM, etc.) is specified in `CPLD-HW-0001` and is **not**
assumed to be a synchronous serial read here. This section defines only the
**image storage format**.

Packed ROM image file (`*.romimg`), little-endian:

```
Offset   Size   Field
0x0000   4      Magic  "CPROM" -> {0x43,0x50,0x52,0x4D,0x31} truncated to 4: 'C','P','R','M'
0x0004   2      Version (0x0001)
0x0006   2      Entry count N
0x0008   N*16   Entry table (see below)
...      ...    Payload regions (each entry points here), 256-byte aligned
```

Each 16-byte entry:

```
Offset Size Field
0x00   1    Kind: 0=SYSTEM, 1=SLOT_ENTRY, 2=SLOT_EXPANSION
0x01   1    Slot number (0 for SYSTEM; 1..7 for slot regions)
0x02   2    Apple CPU base address (e.g. 0xC700, 0xC800, 0x8000)
0x04   4    Logical length in bytes (0x100 entry, up to 0x800 expansion)
0x08   4    Payload file offset
0x0C   4    CRC32 of payload (0 = unchecked)
```

Recommended partitioning (2 KiB granularity in flash for erase alignment):

| Partition | Flash region (example) | Kind | Apple base | Length |
| --- | --- | --- | --- | --- |
| System ROM | `0x000000–0x007FFF` | SYSTEM | `$8000` | `0x8000` |
| Slot 7 entry | `0x008000–0x0083FF` | SLOT_ENTRY | `$C700` | `0x0100` |
| Slot 7 expansion | `0x008400–0x008BFF` | SLOT_EXPANSION | `$C800` | `0x0800` |
| Slots 1–6 entry (optional) | `0x009000–…` | SLOT_ENTRY | `$Cn00` | `0x0100` each |

Conventions:
- **Erased/padding value: `0xFF`** in physical flash; the emulator fills unused
  logical ROM with `0xA0` (its existing idle-bus convention) before applying
  overlays. Overlay payloads shorter than their window are padded with `0xFF`.
- All payloads are 256-byte aligned to match slot-ROM granularity.
- A SLOT_EXPANSION entry is only honoured for the slot that owns the window
  (slot 7 by default); additional slots' expansion images may be stored but are
  not mapped in the emulator's raw-address model (see §1).

## 5. Build/packing tool contract

The emulator and STM32 build share one packer (to be implemented under
`CPLD-ROM-0003` for the emulator path):

- Input: a manifest listing `{kind, slot, apple_base, length, source_file}`.
- Output: a `*.romimg` file as in §4, plus a C header for the emulator build
  (array + entry table) so no runtime file I/O is required in the emulator.
- The packer rejects overlapping logical windows and any entry whose
  `apple_base + length` crosses a window boundary defined in §2.

## 6. Acceptance mapping

| CPLD-ROM-0002 criterion | Where satisfied |
| --- | --- |
| Seven per-slot `Cn00–CnFF` sections + `C800–CFFF` selection + physical offsets | §2, §3, §4 (incl. slot-7 example) |
| No two active ROM windows alias unintended physical bytes; CFFF/slot transitions defined against ROM-0001 | §1, §3; verified by `tb_mmu_addr.cpp` tests 3–5 |
| Layout implementable in emulator and buildable for STM32 flash without changing Apple-visible ROM contents | §3 (overlay-after-load), §4–§5 |

## 7. Open items deferred to other tickets

- Real-time physical ROM-read servicing and slot-6 external controller
  coexistence: `CPLD-HW-0001`.
- Slot-7 ProDOS entry-point and signature byte layout within `$C700–$C7FF`:
  `CPLD-FW-0001`.
- Emulator implementation of the overlay/packing and tests: `CPLD-ROM-0003`.
