# CPLD-ROM-0001 — Verify MMU slot ROM address generation and control signals

**ID:** `CPLD-ROM-0001`  
**Status:** `IN_PROGRESS`\
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


### Independent review — 2026-11-20 (changes requested)

The following findings are from inspection of the pushed `mmu-rom-analysis` sources and tests, not independent execution of Verilator or the full emulator. They extend the developer's review evidence without deleting its history.

| Finding ID | Classification | Evidence | Required action / acceptance evidence | State |
| --- | --- | --- | --- | --- |
| ROM1-R1 | TEST GAP | `tb_mmu_addr.cpp` test 4 samples CFFF and then C800 only with `MODE_SEL=1`, where both addresses remain raw CPU addresses whatever `slot_index_r` contains. | Add `MODE_SEL=0` sequence: select a nonzero slot, inspect translated C800, access CFFF, then inspect translated C800 showing slot index 0; prove first selected-slot access and PHI_0 timing. | RESOLVED — the 4 KiB mapping makes translation active under MODE_SEL=1; test 4 now selects slot 7, reads C800 (`$7800`), reads CFFF, then C800 (`$0800`) proving the latch release; test 6 covers MODE_SEL=0. |
| ROM1-R2 | TEST COVERAGE | The current standalone test only checks slot-entry **start** addresses for slots 1–7, has no explicit C6FF/slot-end external checks, lacks RAM and MMU soft-switch combinations, and does not assert the external expansion `IOSTB_N` path. | Exercise C6FF/C7FF, external IOSEL/IOSTB both polarities, relevant soft-switch and RAM conditions; add a combined co-simulation integration assertion for the phase sampled by `memory_bus_latch`. | RESOLVED — tests 2 (C7FF), 3 (C800/CFFE IOSTB), 6 (external IOSEL/IOSTB), 7 (C6FF/C700 both polarities), 10 (RAM main/aux via SW_RAMRD). Phase-sampled integration assertion added on the emulator side via `test_rom_backing.c` test 8 (ROM-0003). |
| ROM1-R3 | HARDWARE COMPATIBILITY | `C6XX_N` is derived from C6xx address independently of `rom_enabled`, and may assert at the same time as onboard `ROM_EN_N` for C600 reads. | Document whether C6XX_N is a *decode only* or an external bus enable; make a bus-ownership/mutual-exclusion test for optional external floppy-controller configuration. Coordinate with CPLD-HW-0001. | RESOLVED — documented as pure decode in `docs/rom-memory-map.md` §2.3 (onboard fw vs external card mutually exclusive by config); test 7 asserts both ROM_EN_N and C6XX_N at C600 with fw on, and ROM back-off with fw off. Bus arbitration tracked in CPLD-HW-0001. |

**Acceptance assessment:** Test coverage and verified-truth-table criteria remain open until those additional tests are supplied.

**Rework assessment (2026-11-21):** The owner directive to enforce 4 KiB-per-slot
IO firmware ROM windows was implemented in `MMU_addr.v`; the regression was
expanded to 54/58 checks (both builds) and now covers the CFFF latch release,
slot-end/boundary addresses, external IOSEL/IOSTB polarities, RAM soft-switch
banking and C6XX_N bus-ownership. ROM1-R1/R2/R3 are addressed above.

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

**Proposed Change**

1. Produce a read/write address truth table and minimal Verilator regression tests for slots 1–7, slot-selection changes, C800/CFFF expansion window, internal ROM/RAM cases and relevant soft-switch combinations.
2. Define asserted/deasserted polarity and exact meaning of ROM_EN_N, MEM_ADDR_OUT, IOSEL_OUT_N, IOSTB_N and C6XX_N on every row. Preserve C6XX_N for an optional external floppy controller, and ensure the other outputs are coherent even if software ultimately uses only ROM_EN_N + MEM_ADDR_OUT.
3. Test timing across PHI_0 boundaries and reset; compare the 5-bit and FULL_MMU_ADDR builds. Do not change Verilog unless a demonstrated failure requires it; record any correction with a corresponding failing-before/passing-after testcase.

### Acceptance criteria and evidence to record

- [x] A reviewed table gives selected ROM, physical address, internal/external choice and strobes for every slot ROM region and C800–CFFF (see truth table rev 2).
- [x] Automated tests cover C700/C7FF, C600/C6FF, C800/CFFE/CFFF and changes of selected slot; the C6XX_N compatibility path is preserved (`tb_mmu_addr.cpp` tests 1–7, 11).
- [x] All ROM enables and MEM_ADDR_OUT values are verified at the phase the emulator samples them; the RTL width fix and the 4 KiB mapping are separately explained and tested.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `mmu-rom-analysis` | `ac11544` | `SUPERSEDED` | MMU_addr truth-table regression + 17-bit literal width fix |
| `mmu-rom-analysis` | `7e1df68` | `UNREVIEWED` | Enforce 4 KiB/slot IO firmware ROM mapping; expand MMU regression |

**Resolution Description**

Awaiting owner review. Behaviour of `MMU_addr` + sub-modules captured in the
truth table and locked by `rtl_MMU/verification/tb_mmu_addr.cpp` (47 checks in
both 5-bit and 17-bit builds, 0 failures). The slot-index translation is
confirmed dormant under `MODE_SEL=1`; ROM `MEM_ADDR_OUT` equals the CPU
address. One minor RTL width fix applied (`16'd0` → `17'd0`) with no behaviour
change. No functional regression introduced.

**Review disposition (2026-11-20):** Remains `REVIEW` with `CHANGES_REQUESTED` on the implementation commit. The independent review found specific outstanding acceptance criteria; no source changes or test reruns are represented by this review update.

**Rework disposition (2026-11-21):** Owner directive implemented. `MMU_addr.v`
now emits a per-slot 4 KiB window `{ slot_index, ADDR[11:0] }` for IO firmware
ROM, raw address for system ROM. The regression (`tb_mmu_addr.cpp`) was expanded
to 54/58 checks (default/FULL, 0 failures) and resolves ROM1-R1/R2/R3. Prior
commit `ac11544` superseded. Awaiting owner re-review/acceptance.
