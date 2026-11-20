# CPLD-ROM-0002 — Specify slot ROM and expansion ROM flash layout

**ID:** `CPLD-ROM-0002`  
**Status:** `REVIEW`  
**Title:** Specify slot ROM and expansion ROM flash layout

**Description**

Define a durable and collision-free ROM image layout for per-slot firmware and the shared expansion ROM window. The Apple-visible memory windows are not the same as flash offsets; the layout must explicitly map both.

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: [`CPLD-ROM-0001`](CPLD-ROM-0001.md), [`CPLD-HW-0001`](CPLD-HW-0001.md)

### Existing evidence, assumptions and related source

- Apple slot entry ROM: `$C100–$C7FF`, 256 bytes per slot, with slot 7 at `$C700–$C7FF`. Shared expansion window: `$C800–$CFFF`, bank/owner determined by the preceding slot selection, with `$CFFF` release behaviour to verify.
- `rtl_MMU/MMU_slot_ctrl.v` holds the selected slot; `MMU_addr.v` translates into MEM_ADDR_OUT. This is a controlled selection, not a flat linear copy of the CPU Cxxx address.
- Existing emulator ROM backing in `emulator/onboard/memory_controller.c` uses `rom_memory[ROM_SIZE * 2]` and an address mask `& 0xFFFF`; it cannot be treated as proof of a working multi-slot flash allocation.
- ProDOS metadata uses the last bytes of the slot ROM: `$CnFC–$CnFF`; slot 7 metadata must coexist with the boot/driver entry points.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-20 — Implementation:** Authored [`docs/rom-memory-map.md`](../../rom-memory-map.md) specifying Apple CPU windows, the emulator 64 KiB logical ROM space, the physical `*.romimg` flash layout/packing format, and the slot-7 ProDOS reservation.
- **2026-11-20 — Decision:** Grounded the map on the CPLD-ROM-0001 finding that `MEM_ADDR_OUT` equals the raw CPU address under `MODE_SEL=1`; per-slot ROM is addressed by CPU `$Cn00`, and the `$C800–$CFFF` expansion window is a single physical region reserved for slot 7 in the emulator. Hardware multi-card expansion deferred to `CPLD-HW-0001`.
- **2026-11-20 — Status Change:** `OPEN → IN_PROGRESS → REVIEW`; awaiting owner acceptance of the specification.

**Proposed Change**

1. Create `docs/rom-memory-map.md` giving **for each slot 1–7**: Apple address region, how the slot is selected, CPU IOSEL/IOSTB range, MEM_ADDR_OUT representation, ROM_EN_N behaviour, physical flash partition/offset and image filename/build section.
2. Explicitly state whether each selected slot receives one 2 KiB expansion page, multiple banks, or no expansion ROM; budget space, indicate alignment, erased/padding values, and the ROM image packing/build tool format.
3. Specify ownership of `$C800–$CFFF`, `$CFFF` release, bank switch semantics, slot 6 external-controller coexistence, and internal firmware soft-switch selection. Reserve slot 7 for ProDOS boot and IOMEM firmware.
4. Separate flash **image storage** from the **CPU-readable ROM path**; define data source/timing in the hardware topology ticket rather than silently assuming SPI flash can answer a synchronous CPU ROM read.

### Acceptance criteria and evidence to record

- [x] Document has all seven per-slot Cn00–CnFF sections and C800–CFFF selection and physical ROM image offsets, including example slot7 location (`docs/rom-memory-map.md` §2–§4).
- [x] No two active ROM windows alias the same unintended physical bytes; CFFF and slot-selection transitions are defined and tested against ROM-0001 evidence (§1, §3; `tb_mmu_addr.cpp` tests 3–5).
- [x] Image layout can be implemented in the emulator and built for STM32-managed flash without requiring a change to the Apple-visible ROM contents (§3 overlay-after-load, §4–§5 packer contract).

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | _pending_ | `UNREVIEWED` | Add docs/rom-memory-map.md ROM/expansion flash layout spec |

**Resolution Description**

Awaiting owner review. `docs/rom-memory-map.md` specifies the Apple CPU
windows, the emulator logical ROM space, and the physical `*.romimg` flash
layout/packing format, reserving slot 7 for ProDOS. The specification is
grounded on the verified CPLD-ROM-0001 address behaviour and defers hardware
multi-card expansion and real-time ROM servicing to `CPLD-HW-0001`.
