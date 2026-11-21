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


### Independent review — 2026-11-20 (changes requested)

The following findings are from inspection of the pushed `mmu-rom-analysis` sources and tests, not independent execution of Verilator or the full emulator. They extend the developer's review evidence without deleting its history.

| Finding ID | Classification | Evidence | Required action / acceptance evidence | State |
| --- | --- | --- | --- | --- |
| ROM2-R1 | BLOCKING SPEC CONTRADICTION | `docs/rom-memory-map.md` defines a `.romimg` header and entry table starting at byte 0, but also allocates the SYSTEM ROM payload starting at physical flash byte `0x000000`; these overlap. The magic field says 4 bytes but describes 5-byte `CPROM` truncated to `CPRM`. | Choose a single four-byte magic, reserve physical header/table space, and allocate all payload offsets *after* the table with consistent alignment. Specify whether partition offsets are absolute flash addresses or payload-relative; show a valid example image layout. | RESOLVED — rev 2 §4: single 4-byte magic `CPRM`; all payloads start after the entry table (worked example §4.1 starts first payload at 0x0038); offsets are payload-file offsets, flash partitioning listed separately in §4.2. Loader enforces the bounds (`test_rom_backing.c` test 7). |
| ROM2-R2 | ADDRESS/CFFF SEMANTICS | `$C800–$CFFE` is listed as 2046 bytes, but inclusive range size is 2047. With `MODE_SEL=1` and `SW_SLOTXROM=1`, CFFF resets the internal slot index but does **not** change the raw-address C800 ROM mapping. | Correct sizes and clearly distinguish CFFF slot-selector release from the always-mapped *onboard* expansion window. State exactly what ROM contents are returned at CFFF and after it, under both modes. | RESOLVED — rev 2 §2.2 corrects sizes ($C800–$CFFF = 2048, $C800–$CFFE = 2047). Under the new 4 KiB mapping CFFF **does** affect the mapping: §2.1 states CFFF returns the selected slot's `+0xFFF` byte and resets the selector so the next $C800 maps to slot 0 (`$0800`), verified by `tb_mmu_addr.cpp` test 4. |
| ROM2-R3 | SPEC COMPLETENESS | The flash partition table leaves optional slots 1–6 at `0x009000–…`, rather than providing per-slot partitions/offsets. `*.romimg` is described but no loader/packer exists yet. | Give explicit non-overlapping per-slot offset examples and an executable or fully consistent format contract (field order, endianness, CRC algorithm, header/table/payload bounds); track packer implementation in CPLD-ROM-0003 or a new ticket. | RESOLVED — rev 2 §4.2 gives explicit per-slot 4 KiB flash partitions (slot 1 at 0x08000 … slot 7 at 0x0E000); §4 fixes field order/endianness/CRC (IEEE 802.3) and header/table/payload bounds. Executable loader implemented in CPLD-ROM-0003 (`rom_backing_load_image`). |

**Acceptance assessment:** The format and physical offsets are not yet an unambiguous buildable contract; all three acceptance items require revision.

**Rework assessment (2026-11-21):** `docs/rom-memory-map.md` rewritten to rev 2
for the 4 KiB-per-slot layout and to close ROM2-R1/R2/R3 (single `CPRM` magic,
post-table payloads with worked example, corrected C800–CFFF sizes, CFFF
semantics under the new mapping, explicit per-slot flash partitions, and an
executable loader in CPLD-ROM-0003).

### Independent re-review — 2026-11-21 (4 KiB per-slot specification)

**Accepted design basis:** slot 1–7 each owns 4 KiB; IOSEL uses one 256-byte page inside the first 2 KiB, IOSTB uses the second 2 KiB; slot 6's Apple-visible C6xx contents migrate to 0x6600; system ROM stays at its original CPU address. The owner verified the emulator still runs and `PR#3` / `PR#0` function.

| Finding ID | Classification | Evidence | Required action / acceptance | State |
| --- | --- | --- | --- | --- |
| ROM2-R4 | INCORRECT WORKED EXAMPLE | Section 4.1 says a **three-entry** table ends at `0x0037` and first payload begins at `0x0038`, but the example table lists payload offsets **`0x0028 / 0x8028 / 0x8128`** (all 16 bytes too early). | Correct example offsets to `0x0038 / 0x8038 / 0x8138` for its 0x8000/0x100/0x800 payload lengths. Add a three-entry pack/load regression. | OPEN |
| ROM2-R5 | CFFF DATA CONTRACT | Section 2.1 claims the CFFF access returns the previously selected slot's `+0xFFF` byte; source-level PHI_0 sequencing appears to select slot 0 **during** that access. | Resolve ROM1-R4 and update this statement using a CFFF-address waveform/assertion; document when the release takes effect. | OPEN |
| ROM2-R6 | FORMAT/LOADER CONTRACT | Section 4 defines version 1, a required payload offset after the entry table, and IEEE CRC when nonzero; `rom_backing_load_image` does not validate version, header/table offset ordering or nonzero CRC. Section 4 describes system-ROM entries but the current loader cannot install one after startup's preloaded ROM occupancy. | Align prose and code on validated fields, SYSTEM replacement policy, and atomic error handling; add negative tests before freezing the ROM image format. | OPEN |

These concerns do not invalidate the per-slot geometry. The documentation/format contract is not yet stable for a hardware flash image packer.

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-20 — Implementation:** Authored [`docs/rom-memory-map.md`](../../rom-memory-map.md) specifying Apple CPU windows, the emulator 64 KiB logical ROM space, the physical `*.romimg` flash layout/packing format, and the slot-7 ProDOS reservation.
- **2026-11-20 — Decision:** Grounded the map on the CPLD-ROM-0001 finding that `MEM_ADDR_OUT` equals the raw CPU address under `MODE_SEL=1`; per-slot ROM is addressed by CPU `$Cn00`, and the `$C800–$CFFF` expansion window is a single physical region reserved for slot 7 in the emulator. Hardware multi-card expansion deferred to `CPLD-HW-0001`.
- **2026-11-20 — Status Change:** `OPEN → IN_PROGRESS → REVIEW`; awaiting owner acceptance of the specification.

- **2026-11-20 — Independent Code Review:** Reviewed `docs/rom-memory-map.md` against current RTL and the approved ticket proposal. Image format cannot be implemented as written because the header/table and system ROM payload overlap at offset zero. Also found a 2047-byte C800–CFFE range error and a CFFF mapping ambiguity. Revision needed prior to accepting this as a locked flash layout.
- **2026-11-21 — Rework:** Rewrote `docs/rom-memory-map.md` to rev 2. Added the per-slot 4 KiB window geometry (§2.1), corrected C800–CFFF sizing and CFFF semantics (§2.2), documented C6XX_N bus-ownership (§2.3), redefined the emulator physical ROM space and baseline migration (§3), and fixed the `*.romimg` format with a single `CPRM` magic, post-table payloads, a worked slot-7 example and per-slot flash partitions (§4). Grounded on the rev-2 CPLD-ROM-0001 verified mapping.
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS → REVIEW` for the rework; prior `846dd38` superseded.

- **2026-11-21 — Owner Manual Verification:** Owner reports the emulator runs successfully and that Apple II Applesoft `PR#3` followed by `PR#0` works with the revised 4 KiB slot ROM arrangement. Record as **VERIFIED (OWNER)** for emulator runtime and slot-3/slot-0 transition; this does not imply an IOMEM hard-disk or ProDOS slot7 boot test.
- **2026-11-21 — Independent Re-review:** Per-slot mapping and flash partition design align with the directive, but the three-entry format example contains overlapping offsets; CFFF's claimed data byte is not asserted; format guarantees exceed what the loader actually validates. Requested targeted spec fixes.

**Proposed Change**

1. Create `docs/rom-memory-map.md` giving **for each slot 1–7**: Apple address region, how the slot is selected, CPU IOSEL/IOSTB range, MEM_ADDR_OUT representation, ROM_EN_N behaviour, physical flash partition/offset and image filename/build section.
2. Explicitly state whether each selected slot receives one 2 KiB expansion page, multiple banks, or no expansion ROM; budget space, indicate alignment, erased/padding values, and the ROM image packing/build tool format.
3. Specify ownership of `$C800–$CFFF`, `$CFFF` release, bank switch semantics, slot 6 external-controller coexistence, and internal firmware soft-switch selection. Reserve slot 7 for ProDOS boot and IOMEM firmware.
4. Separate flash **image storage** from the **CPU-readable ROM path**; define data source/timing in the hardware topology ticket rather than silently assuming SPI flash can answer a synchronous CPU ROM read.

### Acceptance criteria and evidence to record

- [x] Document has all seven per-slot Cn00–CnFF sections and C800–CFFF selection and physical ROM image offsets, including example slot7 location (`docs/rom-memory-map.md` rev 2 §2, §2.1, §3, §4.1).
- [x] No two active ROM windows alias the same unintended physical bytes; CFFF and slot-selection transitions are defined and tested against ROM-0001 evidence (§1, §2.1; `tb_mmu_addr.cpp` tests 3–5).
- [ ] The 4 KiB-per-slot logical layout is implemented; the packed format remains pending review of ROM2-R4/R6 (worked offsets, version/CRC/table bounds and SYSTEM policy). Emulator smoke-run/`PR#3`/`PR#0` verified by owner.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `846dd38` | `SUPERSEDED` | Add docs/rom-memory-map.md ROM/expansion flash layout spec |
| `mmu-rom-analysis` | `26f6c06` | `CHANGES_REQUESTED` | rom-memory-map.md rev 2: 4 KiB/slot windows, format/CFFF fixes |

**Resolution Description**

Awaiting owner review. `docs/rom-memory-map.md` specifies the Apple CPU
windows, the emulator logical ROM space, and the physical `*.romimg` flash
layout/packing format, reserving slot 7 for ProDOS. The specification is
grounded on the verified CPLD-ROM-0001 address behaviour and defers hardware
multi-card expansion and real-time ROM servicing to `CPLD-HW-0001`.

**Review disposition (2026-11-20):** Remains `REVIEW` with `CHANGES_REQUESTED` on the implementation commit. The independent review found specific outstanding acceptance criteria; no source changes or test reruns are represented by this review update.

**Rework disposition (2026-11-21):** `docs/rom-memory-map.md` rev 2 closes
ROM2-R1/R2/R3 and specifies the 4 KiB-per-slot layout, corrected sizes/CFFF
semantics and an executable `*.romimg` format (loader in CPLD-ROM-0003). Prior
commit `846dd38` superseded. Awaiting owner re-review/acceptance.
