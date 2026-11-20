# CPLD-ROM-0001 — Verify MMU slot ROM address generation and control signals

**ID:** `CPLD-ROM-0001`  
**Status:** `REVIEW`  
**Title:** Verify MMU slot ROM address generation and control signals

**Description**

Review the current MMU address and enable equations before depending on ROM-backed slot firmware. The source contains a 5-bit/17-bit MEM_ADDR_OUT variant, a stateful slot selector and separately exposed IOSEL_OUT_N, IOSTB_N and C6XX_N; their interactions must be verified over a real CPU read sequence rather than inferred from source expressions alone.

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping
- Branch baseline: `rtl-rework`, inspected 2026-11-20; no implementation implied by this ticket
- Dependencies: None (may start independently)

### Existing evidence, assumptions and related source

- `rtl_MMU/MMU_addr.v`: `mem_addr_out_reg`, `ROM_EN_N = ~rom_enabled & onboard_fw_n`, `IOSEL_N = rom_enabled | ~(IOSEL_INT_map & C6XX_N)`, `IOSTB_N = rom_enabled | ~IOSTB_INT_map`, and `MODE_SEL` influence.
- `rtl_MMU/MMU_slot_ctrl.v`: `slot_index_r` and `rom_fw_enable_r` update on PHI_0, depend on soft switches, and encounter `CFFF_INT`; examine any first-access/off-by-one or bank-latch effects.
- `rtl_MMU/MMU_mem_map.v` and `rtl_MMU/MMU_mem_select.v`: inspect C1xx–C7xx, C800–CFFE, CFFF, C0xx and Dxxx–Fxxx paths.
- `rtl_IOU_MMU_TB/IOU_MMU_tb.v`: test actual output pin semantics; C6XX_N is a dedicated output for an optional real slot-6 controller.
- Known risk, not confirmed defect: default MEM_ADDR_OUT is only bits [16:12], and `FULL_MMU_ADDR` changes the HDL interface. The C++ adapter reconstructs lower 12 address bits in the default build.

### Verified behaviour and truth table (2026-11-20)

Verified by Verilator regression `rtl_MMU/verification/tb_mmu_addr.cpp`
(driven over real PHI_0 cycles, sampled while PHI_0 is high, matching the
emulator's `cpld/rtl_memory_bus.cpp` timing). Values below are **confirmed**
by automated test in the `FULL_MMU_ADDR` build unless noted; `MODE_SEL=1`
(the emulator configuration) unless stated otherwise.

| CPU access | Condition | ROM_EN_N | RAM_EN_N | IOSEL_N | IOSTB_N | C0XX_N | C6XX_N | MEM_ADDR_OUT |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `C100`–`C700` read | SLOTXROM=1 | 0 | 1 | 1 | 1 | 1 | 1 (`C600`→0) | = CPU addr |
| `C800`/`CFFE` read | after slot select | 0 | 1 | 1 | 1 | 1 | 1 | = CPU addr |
| `CFFF` read | SLOTXROM=1 | 0 | 1 | 1 | 1 | 1 | 1 | `0xCFFF`; slot selector → 0 |
| `C080` read | I/O page | 1 | 1 | 1 | 1 | 0 | 1 | 0 |
| `D000`/`F800` read | system ROM | 0 | 1 | 1 | 1 | 1 | 1 | = CPU addr |
| `C500` read | SLOTXROM=0, MODE_SEL=1 | 1 | 1 | 0 | 1 | 1 | 1 | 0 (external card drives bus) |
| `C500` read | SLOTXROM=0, MODE_SEL=0 | 0 | 1 | 0 | 1 | 1 | 1 | `0x5500` (slot-translated) |
| `C600` read | SLOTXROM=0 | 1 | 1 | 1 | 1 | 1 | 0 | 0 (external slot-6) |
| `C700` **write** | SLOTXROM=1 | 1 | 1 | 0 | 1 | 1 | 1 | 0 (no ROM on write) |

**Primary finding — the slot-index translation is dormant in the emulator.**
With `MODE_SEL=1` (forced by `cpld/rtl_cpu_bus.cpp`), `onboard_fw_n` is always
high, so the `~onboard_fw_n` branch of `MMU_addr` that produces the translated
address `{2'b00, slot_index, ADDR[11:0]}` is **never** selected. Firmware ROM
reads take the `rom_enabled` branch, so **`MEM_ADDR_OUT` equals the raw CPU
address** (bit 16 = 0). The stateful slot selector (`slot_index_r`) therefore
does **not** translate ROM addresses in the emulator; it only matters for the
external-card path (`MODE_SEL=0`, slot firmware disabled), where the translated
address and `ROM_EN_N` are produced as shown. This is the critical input to
`CPLD-ROM-0002`/`-0003`: per-slot ROM images are addressed by CPU `$Cxxx`,
not by a slot-translated offset.

**No off-by-one in slot latching.** Because there is exactly one PHI_0 rising
edge per CPU cycle and the outputs are sampled after it, the slot latched from
`ADDR[10:8]` on an IOSEL access is already effective for that same access.
`CFFF` resets the selector to 0 (verified).

**Minor RTL correction.** `MMU_addr.v` assigned a 16-bit literal (`16'd0`) to
the 17-bit `mem_addr_out_reg` in the `FULL_MMU_ADDR` default branch, which
`verilator -Wall` rejects (blocks building the full-address variant). Changed
to `17'd0`; behaviour is unchanged (the branch already produced zero).

### Review Findings

| Finding ID | Classification | Evidence | Required action / acceptance evidence | State |
| --- | --- | --- | --- | --- |
| RF-1 | VERIFICATION_REQUIRED | Truth table above; `tb_mmu_addr.cpp`, 47 checks ×2 builds pass | Owner review of locked behaviour | VERIFIED (awaiting acceptance) |
| RF-2 | CORRECTION_REQUIRED | `16'd0` into 17-bit reg blocked `-Wall` full build | Width literal fixed to `17'd0`; full build now passes | VERIFIED |

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-20 — Implementation:** Added `rtl_MMU/verification/tb_mmu_addr.cpp` and `run_tests.sh`, a Verilator regression covering slots 1–7 IOSEL, C800/CFFE/CFFF expansion, CFFF release, C0XX, system ROM, external-card translation (MODE_SEL=0), C6XX_N and write cycles.
- **2026-11-20 — Test:** Both `default` (5-bit) and `FULL_MMU_ADDR` (17-bit) builds pass: 47 checks each, 0 failures (Verilator 5.045).
- **2026-11-20 — Investigation:** Confirmed the slot-index translation branch is unused when `MODE_SEL=1`; ROM `MEM_ADDR_OUT` = raw CPU address. Recorded in the truth table above and flagged for ROM-0002/0003.
- **2026-11-20 — Rework:** Fixed `MMU_addr.v` 17-bit literal width to unblock `-Wall` full-address build (behaviour preserved).
- **2026-11-20 — Status Change:** `OPEN → IN_PROGRESS → REVIEW` after tests passed; awaiting owner acceptance.

**Proposed Change**

1. Produce a read/write address truth table and minimal Verilator regression tests for slots 1–7, slot-selection changes, C800/CFFF expansion window, internal ROM/RAM cases and relevant soft-switch combinations.
2. Define asserted/deasserted polarity and exact meaning of ROM_EN_N, MEM_ADDR_OUT, IOSEL_OUT_N, IOSTB_N and C6XX_N on every row. Preserve C6XX_N for an optional external floppy controller, and ensure the other outputs are coherent even if software ultimately uses only ROM_EN_N + MEM_ADDR_OUT.
3. Test timing across PHI_0 boundaries and reset; compare the 5-bit and FULL_MMU_ADDR builds. Do not change Verilog unless a demonstrated failure requires it; record any correction with a corresponding failing-before/passing-after testcase.

### Acceptance criteria and evidence to record

- [x] A reviewed table gives selected ROM, physical address, internal/external choice and strobes for every slot ROM region and C800–CFFF (see truth table).
- [x] Automated tests cover C700/C7FF, C600/C6FF, C800/CFFE/CFFF and changes of selected slot; the C6XX_N compatibility path is preserved (`tb_mmu_addr.cpp` tests 1–6, 10).
- [x] All ROM enables and MEM_ADDR_OUT values are verified at the phase the emulator samples them; the one RTL defect (17-bit literal width) is separately explained and tested.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | _pending_ | `UNREVIEWED` | MMU_addr truth-table regression + 17-bit literal width fix |

**Resolution Description**

Awaiting owner review. Behaviour of `MMU_addr` + sub-modules captured in the
truth table and locked by `rtl_MMU/verification/tb_mmu_addr.cpp` (47 checks in
both 5-bit and 17-bit builds, 0 failures). The slot-index translation is
confirmed dormant under `MODE_SEL=1`; ROM `MEM_ADDR_OUT` equals the CPU
address. One minor RTL width fix applied (`16'd0` → `17'd0`) with no behaviour
change. No functional regression introduced.
