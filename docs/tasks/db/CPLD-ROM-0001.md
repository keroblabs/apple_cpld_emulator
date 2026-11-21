# CPLD-ROM-0001 — Verify MMU slot ROM address generation and control signals

**ID:** `CPLD-ROM-0001`  
**Status:** `REVIEW`\
**Title:** Verify MMU slot ROM address generation and control signals

**Description**

Verify that the MMU selects a private 4 KiB window for each card ROM. The
Apple IOSEL page must read from the first 256 bytes of that window and IOSTB
must read from the upper 2 KiB. Preserve the original Apple ROM and slot-6
decode while exercising slot selection and release at the emulator's bus
sampling phase.

**Details**

- Kind: TASK
- Category: `CPLD-ROM`
- Milestone: M1 — ROM mapping
- Branch baseline: `mmu-rom-analysis` at `ffb534d`
- Dependencies: None. Physical bus arbitration is tracked in [CPLD-HW-0001](CPLD-HW-0001.md).

### Address and strobe contract

`MMU_slot_ctrl` latches the IOSEL slot on the rising `PHI_0` edge. The
emulator captures the decoded address and pins in the settled high phase.
Internal Apple reads (`rom_enabled=1`) use the CPU address. Card IOSEL uses
`(slot_index << 12) | ADDR[7:0]`. Card IOSTB uses
`(slot_index << 12) | ADDR[11:0]`. The card selected at `$CFFF` supplies
that cycle's byte. The next cycle uses slot 0 unless IOSEL selects a new slot.
Reading or writing `$CFFF` triggers the release.

| Transfer | Mode / selection | ROM_EN_N | RAM_EN_N | IOSEL_N | IOSTB_N | C6XX_N | Physical address |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `$C700–$C7FF` read | 1 / card | 1 | 1 | 0 | 1 | 1 | `$7000–$70FF` |
| `$C600–$C6FF` read | 1 / card | 1 | 1 | 1 | 1 | 0 | `$6000–$60FF` |
| `$C800–$CFFF` read | 1 / card, slot 7 selected | 1 | 1 | 1 | 0 | 1 | `$7800–$7FFF` |
| `$C800` read after `$CFFF` | 1 / card, slot 0 | 1 | 1 | 1 | 0 | 1 | `$0800` |
| `$C5AB` read | 0 / card | 0 | 1 | 0 | 1 | 1 | `$50AB` |
| `$C6FF` read | 0 / external slot 6 | 1 | 1 | 1 | 1 | 0 | `$60FF` |
| `$C800` read after selecting slot 6 | 0 / card | 0 | 1 | 1 | 0 | 1 | `$6800` |
| `$CFFF` write | Either / release | 1 | 1 | 1 | 0 | 1 | — |
| `$C600` read | Either / internal Cx | 0 | 1 | 1 | 1 | 0 | `$C600` |
| `$C800` read after `$CFFF` | Either / `SW_SLOTXROM=1` | 0 | 1 | 1 | 1 | 1 | `$C800` |
| `$C080` read | Either / I/O, `C0XX_N=0` | 1 | 1 | 1 | 1 | 1 | — |
| `$D000–$FFFF` read | Either / system ROM | 0 | 1 | 1 | 1 | 1 | CPU address |
| `$2000` read | Either / main or auxiliary RAM | 1 | 0 | 1 | 1 | 1 | `$2000` or `$12000` |

The full MMU outputs the complete address. The 5-bit MMU outputs bits
`[16:12]`. The emulator reconstructs the offset from the CPU address and
sampled ROM and slot strobes, including the dedicated slot-6 decode. The
[ROM map](../../rom-memory-map.md) gives all seven slot ranges.

### Review findings and current evidence

| Finding | Evidence for re-review |
| --- | --- |
| ROM1-R1, slot release and phase | The RTL regression checks both the current `$CFFF` address and the following `$C800` in both modes. It also checks consecutive `$CFFF` reads and immediate IOSEL reselection. The combined bus test reads the selected slot's final byte and then zero from the released slot window. |
| ROM1-R2, address and switch coverage | Both builds cover all seven IOSEL starts and ends, IOSTB boundaries, cross-slot selection, C0xx, RAM banking and language-card/system ROM. Combined tests exercise real IOU soft switch writes and the scheduler's settled `PHI_0` capture. |
| ROM1-R3, external slot-6 compatibility | `C6XX_N` asserts for internal and external C6xx reads. On an external C6xx read, `ROM_EN_N` stays high in both modes. The emulator's disk I/O handoff at `$C0EC` is tested. Physical bus ownership remains a CPLD-HW-0001 deliverable. |

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.
- **2026-11-20 — Implementation:** Added `rtl_MMU/verification/tb_mmu_addr.cpp` and `run_tests.sh`, a Verilator regression covering slots 1–7 IOSEL, C800/CFFE/CFFF expansion, CFFF release, C0XX, system ROM, external-card translation (MODE_SEL=0), C6XX_N and write cycles.
- **2026-11-20 — Test:** Both `default` (5-bit) and `FULL_MMU_ADDR` (17-bit) builds pass: 47 checks each, 0 failures (Verilator 5.045).
- **2026-11-20 — Investigation:** Confirmed the slot-index translation branch is unused when `MODE_SEL=1`; ROM `MEM_ADDR_OUT` = raw CPU address. Recorded in the truth table above and flagged for ROM-0002/0003.
- **2026-11-20 — Rework:** Fixed `MMU_addr.v` 17-bit literal width to unblock `-Wall` full-address build (behaviour preserved).
- **2026-11-20 — Status Change:** `OPEN → IN_PROGRESS → REVIEW` after tests passed; awaiting owner acceptance.

- **2026-11-20 — Independent Code Review:** Source-level review of commits `ac115446` and follow-up ticket hash recording. The 17-bit width fix is appropriate. The 47×2 checks are valid as reported by the developer but do not prove the CFFF latch release: test 4 only compares raw addresses under MODE_SEL=1. Requested additional external-slot transition and decode coverage. No code executed by reviewer.
- **2026-11-21 — Rework (owner directive):** Enforced 4 KiB-per-slot IO firmware ROM windows in `rtl_MMU/MMU_addr.v` — the `rom_enabled` branch now splits on `D_FXXX_INT`: system ROM stays raw, IO firmware ROM (IOSEL/IOSTB) uses `{ slot_index, ADDR[11:0] }` (first 2 KiB IOSEL, second 2 KiB IOSTB). The external-card path shares the same formula.
- **2026-11-21 — Test:** Rewrote `tb_mmu_addr.cpp` to lock the 4 KiB mapping and close ROM1-R1/R2/R3: CFFF latch release now observable (test 4), IOSEL/IOSTB boundaries (2,3), cross-slot ownership (5), external polarities (6), C6XX_N bus-ownership (7), system ROM raw (9), RAM main/aux banking (10). Results: default 54/0, FULL 58/0 (Verilator 5.045).
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS → REVIEW` for the rework; prior `ac11544` superseded by the rev-2 commit.
- **2026-11-21 — Status Change:** `REVIEW → IN_PROGRESS` on owner direction to verify the address decode against the raw 4 KiB slot binary layout.
- **2026-11-21 — Discovery:** The baseline Verilator regression passes 54 checks in the 5-bit build and 58 in the full build. The address within the first 2 KiB still includes the slot's CPU page bits, so it misses a raw binary's entry page at offset zero. The CFFF test checks the address of the following cycle. Reproduce the CFFF return address and distinguish the internal Apple Cx ROM from card ROM before changing the decoder.
- **2026-11-21 — Regression reproduced:** The new `tb_mmu_addr.cpp` produced 47 failures in the 5-bit build and 58 in the full build against the original RTL. For example, hardware mode mapped `$C5AB` to `$55AB` and `$CFFF` to slot 0's `$0FFF`. It also asserted ROM on card writes. The old tests passed while missing these cases.
- **2026-11-21 — Implementation:** `MMU_addr.v` normalizes IOSEL to the start of each 4 KiB window, retains the Apple ROM's original Cx offsets under its soft switches and provides the card address in both modes. `MMU_slot_ctrl.v` defers the `$CFFF` release until the next cycle and `MMU_mem_select.v` wires the updated selector. `rtl_interconnect.cpp` reconstructs the same offsets for the 5-bit model.
- **2026-11-21 — Verification:** `bash rtl_MMU/verification/run_tests.sh` passes 462 checks per variant. `bash emulator/onboard/verification/run_tests.sh` passes 18,464 combined bus checks per variant. Both use Verilator 5.045 (v5.044-96-g30e6cd909).
- **2026-11-21 — Status Change:** `IN_PROGRESS → REVIEW` with the failing-before and passing-after regressions recorded.

**Proposed Change**

Use a raw 4 KiB window per slot, with the IOSEL page at offsets
`$000–$0FF` and the IOSTB page at `$800–$FFF`. The original Apple
ROM and these windows share the 256 KiB address space specified in
[CPLD-ROM-0002](CPLD-ROM-0002.md).

- [x] Pin and address regression passes in both 5-bit and 17-bit MMU builds.
- [x] First slot access, cross-slot selection and `$CFFF` read/write release are verified at the production sampling phase.
- [x] Internal Apple Cx and system ROM, RAM banking and the slot-6 decode are exercised.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `ac11544` | `SUPERSEDED` | MMU_addr truth-table regression + 17-bit literal width fix |
| `mmu-rom-analysis` | `7e1df68` | `SUPERSEDED` | Enforce 4 KiB/slot IO firmware ROM mapping; expand MMU regression |
| `mmu-rom-analysis` | `1965baa` | `UNREVIEWED` | Record approved raw-ROM scope and start rework |

**Resolution Description**

The raw layout is implemented and verified on the emulator and both RTL
variants. Awaiting owner review. The slot-6 physical controller configuration
is tracked in CPLD-HW-0001.
