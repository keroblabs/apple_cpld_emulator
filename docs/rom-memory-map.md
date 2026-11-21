# ROM and Expansion-ROM Memory Map (CPLD-ROM-0002)

**Final ROM release constraint:** [Single DIP32 raw flash image contract](rom-flash-image-contract.md) — one headerless binary, also loaded unchanged by the emulator. The `*.romimg` material below describes an **optional development container only**, not programmer data.

**Status:** Specification for review — rev 2 logical address map; final binary requirement added 2026-11-21 (branch `mmu-rom-analysis`).
**Methodology:** [Task and Bug Tracking Methodology](tasks/bug-tracking-methodology.md).
**Owning ticket:** [`CPLD-ROM-0002`](tasks/db/CPLD-ROM-0002.md).
**Depends on:** [`CPLD-ROM-0001`](tasks/db/CPLD-ROM-0001.md) (verified address behaviour),
[`CPLD-HW-0001`](tasks/db/CPLD-HW-0001.md) (physical ROM-read topology).

This document defines a durable, collision-free ROM image layout for per-slot
firmware and the expansion-ROM window, mapping the **Apple-visible memory
windows** to **physical image / flash offsets**. Two address spaces are kept
strictly separate:

1. **Apple CPU address space** — the `$Cxxx`/`$Dxxx`–`$Fxxx` windows the 65C816
   presents on `ADDR[15:0]`.
2. **Physical ROM byte-address space** — the byte locations returned by the real DIP32 ROM (and the emulator in raw-image mode). A metadata-bearing `*.romimg` development container has separate *file offsets*; the STM32's serial flash is separate storage.

> **Rev 2 — key change (owner directive).** IO firmware ROM is organised into a
> **4 KiB window per slot**. The first 2 KiB of each window holds the slot's
> IOSEL entry ROM (256 bytes used), and the second 2 KiB holds that slot's
> IOSTB expansion ROM. This supersedes rev 1's single shared expansion region.

## 1. Grounding fact from CPLD-ROM-0001 (verified)

The MMU address behaviour is verified by
[`rtl_MMU/verification/tb_mmu_addr.cpp`](../rtl_MMU/verification/tb_mmu_addr.cpp)
(58 checks in the `FULL_MMU_ADDR` build, 54 in the 5-bit build, 0 failures).

> For **IO firmware ROM** (IOSEL/IOSTB with onboard firmware enabled), the MMU
> emits `MEM_ADDR_OUT = { slot_index, ADDR[11:0] }` — a per-slot 4 KiB window.
> For **system ROM** (`$D000-$FFFF`, `D_FXXX`), `MEM_ADDR_OUT` is the raw CPU
> address. This holds in both `MODE_SEL=1` (emulator) and the external-card
> path (`MODE_SEL=0`), so the `$CFFF` slot-selector release is observable: the
> next expansion access maps into slot 0's window.

Consequences for this map:

- Per-slot IO firmware is addressed by `{ slot_index, ADDR[11:0] }`, so each
  slot owns a private 4 KiB physical window; no two slots alias.
- Because the within-window offset is exactly `ADDR[11:0]`, the default 5-bit
  build reconstructs the full physical address as `(MEM_ADDR_OUT[4:0] << 12) |
  (cpu_address & 0xFFF)` with **no** special-case logic in the C++ interconnect.

## 2. Apple CPU address windows

| Apple window | Size | Role | Selected by | ROM_EN_N | MEM_ADDR_OUT |
| --- | --- | --- | --- | --- | --- |
| `$C000–$C0FF` | 256 B | Soft switches / I/O (incl. C0Fx mailbox) | `C0XX_N` | 1 (not ROM) | — |
| `$C100–$C1FF` | 256 B | Slot 1 IOSEL entry ROM | IOSEL, `SW_SLOTXROM` | 0 | `{1, $100}` = `$1100` |
| `$C200–$C2FF` | 256 B | Slot 2 IOSEL entry ROM | IOSEL, `SW_SLOTXROM` | 0 | `{2, $200}` = `$2200` |
| `$C300–$C3FF` | 256 B | Slot 3 IOSEL entry ROM | IOSEL, `SW_SLOTXROM`/`SW_SLOT3ROM` | 0 | `{3, $300}` = `$3300` |
| `$C400–$C4FF` | 256 B | Slot 4 IOSEL entry ROM | IOSEL, `SW_SLOTXROM` | 0 | `{4, $400}` = `$4400` |
| `$C500–$C5FF` | 256 B | Slot 5 IOSEL entry ROM | IOSEL, `SW_SLOTXROM` | 0 | `{5, $500}` = `$5500` |
| `$C600–$C6FF` | 256 B | Slot 6 IOSEL entry ROM (Disk II / ext.) | IOSEL, `SW_SLOTXROM`; `C6XX_N` for ext. | 0 (onboard) | `{6, $600}` = `$6600` |
| `$C700–$C7FF` | 256 B | **Slot 7 IOSEL entry ROM (ProDOS boot)** | IOSEL, `SW_SLOTXROM` | 0 | `{7, $700}` = `$7700` |
| `$C800–$CFFF` | 2 KiB | IOSTB expansion ROM of the **selected** slot | IOSTB after a slot IOSEL; selector reset by `$CFFF` | 0 | `{slot, ADDR[11:0]}`, i.e. `$x800–$xFFF` |
| `$D000–$DFFF` | 4 KiB | System ROM (bank-switched / LC) | `D_FXXX_INT` | 0 | `$D000–$DFFF` (raw) |
| `$E000–$FFFF` | 8 KiB | System ROM (monitor/BASIC) | `D_FXXX_INT` | 0 | `$E000–$FFFF` (raw) |

### 2.1 Per-slot 4 KiB window geometry

Each slot `N` (1–7) owns physical window `[N*0x1000, N*0x1000 + 0xFFF]`:

```
  N*0x1000 +0x000 ┌───────────────────────────┐
                  │ first 2 KiB (IOSEL region) │
      +(N<<8)     │   IOSEL entry page:        │  Apple $Cn00–$CnFF
                  │   256 bytes at +(N<<8)     │  → phys N*0x1000 + N*0x100
      +0x7FF      │   (rest reserved / $FF)    │
  N*0x1000 +0x800 ├───────────────────────────┤
                  │ second 2 KiB (IOSTB region)│  Apple $C800–$CFFF
      +0xFFF      │   2 KiB expansion ROM      │  → phys N*0x1000 + 0x800..0xFFF
                  └───────────────────────────┘
```

- The IOSEL page lands at offset `N<<8` inside the first 2 KiB because the MMU
  passes `ADDR[11:0]` unchanged and `ADDR[11:8] = {0, N}` for `$Cn00`. This is a
  direct, hardware-realisable consequence of the decode; the position is
  deterministic per slot.
- `$CFFF` is part of the IOSTB region and is a normal firmware-ROM byte
  (`phys {slot, 0xFFF}`); reading it also resets the internal slot selector to
  0, so a **subsequent** `$C800` access maps to slot 0's window (`$0800`). The
  byte returned *at* `$CFFF` is the selected slot's `+0xFFF` byte.

### 2.2 `$C800–$CFFF` size (correcting rev-1 ROM2-R2)

`$C800`–`$CFFF` inclusive is **2048 bytes** (`0xCFFF − 0xC800 + 1 = 0x800`).
`$C800`–`$CFFE` is 2047 bytes; `$CFFF` is the final byte of the region, not a
gap. Rev 1's "2046 bytes" was incorrect.

### 2.3 `C6XX_N` and external slot-6 coexistence (ROM1-R3)

`C6XX_N` is a **pure address decode** of `$C6xx` IOSEL and is independent of
`rom_enabled`. With onboard slot-6 firmware **enabled** (`SW_SLOTXROM=1`), both
`ROM_EN_N` and `C6XX_N` assert at `$C600`. Therefore the onboard slot-6 ROM and
an external slot-6 controller are **mutually exclusive by configuration**:
disable onboard slot-6 firmware (external card drives the bus and owns
`C6XX_N`), or do not fit an external card (onboard firmware serves `$C600`).
Bus-ownership arbitration and level shifting are tracked in `CPLD-HW-0001`.

## 3. Emulator physical ROM space (CPU-readable path)

The emulator exposes a **64 KiB logical ROM space** indexed by the MMU physical
address, partitioned as:

| Physical range | Size | Contents |
| --- | --- | --- |
| `$0000–$7FFF` | 32 KiB | **IO firmware ROM** — eight 4 KiB per-slot windows (slot 0 unused) |
| `$1000–$7FFF` | — | slots 1–7 windows (`N*0x1000`), geometry per §2.1 |
| `$8000–$CFFF` | 20 KiB | System ROM image body / reserved (not a CPU ROM window) |
| `$D000–$FFFF` | 12 KiB | System ROM (monitor/BASIC/LC), read raw at `$D000–$FFFF` |

The emulator loads `apple_rom_bin` (32 KiB) at physical `$8000`, covering
`$8000–$FFFF`, so the system ROM window `$D000–$FFFF` is populated directly.

**Baseline preservation.** To keep Apple-visible bytes identical to the previous
raw-address behaviour (and so not regress boot or the slot-6 Disk II ROM), the
emulator migrates the internal `$C100–$CFFF` firmware from `apple_rom_bin` into
each per-slot window as a *baseline* at startup
(`memory_controller_init` → `rom_backing_fill_baseline`):

- Slot `N` IOSEL page ← `apple_rom_bin[$Cn00]`, 256 bytes, at `phys N*0x1100`.
- Slot `N` IOSTB expansion ← `apple_rom_bin[$C800]`, 2 KiB, at `phys N*0x1000+0x800`.

Baselines are **not** occupancy-marked, so a packaged image (below) may override
any slot window. Explicit installs are overlap-checked against each other and
against the system region.

## 4. Development container: packed `*.romimg` (NOT the DIP32 programmer image)

**Release distinction:** The `CPRM` header/table format described in this section is a *development container*. The physical DIP32 device must instead be programmed with the **single raw, headerless binary** defined in [the DIP32 flash image contract](rom-flash-image-contract.md). In that final binary, file offset = physical ROM address, with no container/header/table or per-slot file-offset translation. The emulator must load that identical raw binary in dedicated raw-flash mode. Section 4.2's STM32 serial-flash offset examples must not be used as raw DIP32 ROM offsets.

Current implementation of this development container is under review: version, checksum and table/payload validation, all-or-nothing installs, and SYSTEM replacement policy are outstanding in `CPLD-ROM-0003`.

Single contiguous image; **little-endian**. The header and entry table occupy
the start of the file; **all payloads follow the table** (no overlap with
offset 0):

```
Offset        Size      Field
0x0000        4         Magic: 'C','P','R','M'  (exactly four bytes)
0x0004        2         Version (0x0001)
0x0006        2         Entry count N
0x0008        N*16      Entry table (16 bytes each)
0x0008+N*16   ...       Payload regions (referenced by entry payload offsets)
```

Each 16-byte entry:

```
Offset Size Field
0x00   1    Kind: 0 = SYSTEM, 1 = SLOT_ENTRY (IOSEL), 2 = SLOT_EXPANSION (IOSTB)
0x01   1    Slot number (0 for SYSTEM; 1..7 for slot regions)
0x02   2    Apple CPU base address (informational, e.g. 0xC700, 0xC800, 0x8000)
0x04   4    Payload length in bytes
0x08   4    Payload file offset (must be >= end of entry table)
0x0C   4    CRC32 of payload (0 = unchecked; algorithm: standard IEEE 802.3)
```

Validation enforced by the loader
([`rom_backing_load_image`](../emulator/onboard/rom_backing.c)):

- Magic must be `CPRM`; truncated headers are rejected.
- `payload_offset + length <= image_len` for every entry.
- Region geometry is enforced via `rom_backing_install_region`:
  - `SLOT_ENTRY`: slot 1–7, `length <= 256`, within the first 2 KiB of the slot
    window.
  - `SLOT_EXPANSION`: slot 1–7, `length <= 2048`, in the second 2 KiB.
  - `SYSTEM`: installed at physical `$8000`, `length <= 32768`.
- Overlapping explicit installs are rejected (256-byte-page occupancy map).

### 4.1 Worked example image (slot 7 ProDOS)

```
Region          Kind            Slot  Apple base  Phys install  Length  Payload off
System ROM      SYSTEM          0     $8000       $8000         0x8000  0x0028
Slot 7 entry    SLOT_ENTRY      7     $C700       $7700         0x0100  0x8028
Slot 7 expand   SLOT_EXPANSION  7     $C800       $7800         0x0800  0x8128
```

With `N = 3` entries, the table spans `0x0008–0x0037` (3×16 = 0x30 bytes), so the
first payload starts at `0x0038` — strictly after the table (no offset-0
overlap, resolving ROM2-R1). Partition/payload offsets above are **payload-file
offsets** (absolute within the `.romimg`), not flash addresses.

### 4.2 Historical STM32 serial-flash partition examples (NOT the DIP32 ROM map)

These example offsets refer **only** to hypothetical STM32-managed serial-flash storage; they are **not** the offsets used by the directly addressed DIP32 ROM. The DIP32's raw byte map is 0x0000–0xFFFF as specified in the single-image contract. The final ROM bus topology and part number remain in `CPLD-HW-0001`.
Suggested erase-aligned (4 KiB) flash partitions, per slot:

| Partition | Flash offset (example) | Kind | Apple base | Max length |
| --- | --- | --- | --- | --- |
| System ROM | `0x00000` | SYSTEM | `$8000` | `0x8000` |
| Slot 1 window | `0x08000` | ENTRY+EXP | `$C100`/`$C800` | `0x1000` |
| Slot 2 window | `0x09000` | ENTRY+EXP | `$C200`/`$C800` | `0x1000` |
| … | … | … | … | … |
| Slot 7 window | `0x0E000` | ENTRY+EXP | `$C700`/`$C800` | `0x1000` |

Erased/padding value in physical flash is `0xFF`; the emulator fills unused
logical ROM with `0xA0` (idle-bus convention) before applying baselines/overlays.

### 4.3 Release output — one physical DIP32 flash image

Build one `apple_cpld_flash.bin`-style raw file from the system ROM and all slot entry/expansion sources. For the present MMU's 16-bit decoded ROM map, **each offset in `0x0000–0xFFFF` corresponds exactly to that physical ROM address** (64 KiB). This file contains slot 6's migrated C6xx bytes at `0x6600`, slot 7's 256-byte entry at `0x7700`, its expansion at `0x7800`, and the system ROM at `0x8000` onwards. Every unused byte has a deterministic fill value. The target DIP32 chip's actual capacity, A16/high-address wiring and programmer image size must be verified in `CPLD-HW-0001`; if larger than the decoded space, specify the high-bank contents and produce a single full-device image as required.

The **same raw bytes** must be the sole emulator ROM contents in release-test mode; the existing `memory_controller_init()` baseline migration/`*.romimg` overlays cannot count as a release-image test. Retain a separate SHA-256 manifest and compare to complete-chip programmer readback when physical hardware is available. Implementation and acceptance: [`CPLD-ROM-0004`](tasks/db/CPLD-ROM-0004.md).

## 5. ProDOS slot-7 metadata

ProDOS firmware metadata occupies the last four bytes of the slot entry ROM,
`$CnFC–$CnFF`. For slot 7 that is `$C7FC–$C7FF`, i.e. physical `$77FC–$77FF`
within the slot-7 entry page. These must coexist with the boot/driver entry
points packed into `$C700–$C7FB` (`$7700–$77FB`); layout detail in
`CPLD-FW-0001`.

## 6. Acceptance mapping

| CPLD-ROM-0002 criterion | Where satisfied |
| --- | --- |
| Seven per-slot `Cn00–CnFF` sections + `C800–CFFF` selection + physical offsets, slot-7 example | §2, §2.1, §3, §4.1 |
| No two active ROM windows alias unintended physical bytes; CFFF/slot transitions defined and tested against ROM-0001 | §1, §2.1; `tb_mmu_addr.cpp` tests 3–5 |
| Buildable image format with consistent header/table/payload bounds, alignment, endianness, CRC | §4 (loader-enforced; `test_rom_backing.c` test 7) |
| Layout implementable in emulator without changing Apple-visible ROM contents | §3 baseline migration; emulator smoke-run clean |

## 7. Items deferred to other tickets

- Real-time physical ROM-read servicing and slot-6 external-controller bus
  ownership: `CPLD-HW-0001`.
- Slot-7 ProDOS entry-point/signature byte layout within `$C700–$C7FF`:
  `CPLD-FW-0001`.
- Development-container `*.romimg` loader: `CPLD-ROM-0003` (implemented, outstanding validation review). **Single raw programmer binary and exact emulator raw mode:** `CPLD-ROM-0004` (not yet implemented).
