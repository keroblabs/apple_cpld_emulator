# CPLD-ROM-0001 — Verify MMU slot ROM address generation and control signals

**ID:** `CPLD-ROM-0001`  
**Status:** `OPEN`  
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

**Notes**

- **2026-11-20 — Discovery:** Planned with project owner as part of the slot7 ProDOS hard-disk programme. Source-level observations are recorded above; candidate designs and hardware assumptions are not approved implementation results.
- **2026-11-20 — Scope:** The STM32 owns the SPI master, RTC and serial flash; no additional CPLD-side SPI master is planned. Preserve slot6 compatibility and respect the existing Apple C0Fx mailbox.

**Proposed Change**

1. Produce a read/write address truth table and minimal Verilator regression tests for slots 1–7, slot-selection changes, C800/CFFF expansion window, internal ROM/RAM cases and relevant soft-switch combinations.
2. Define asserted/deasserted polarity and exact meaning of ROM_EN_N, MEM_ADDR_OUT, IOSEL_OUT_N, IOSTB_N and C6XX_N on every row. Preserve C6XX_N for an optional external floppy controller, and ensure the other outputs are coherent even if software ultimately uses only ROM_EN_N + MEM_ADDR_OUT.
3. Test timing across PHI_0 boundaries and reset; compare the 5-bit and FULL_MMU_ADDR builds. Do not change Verilog unless a demonstrated failure requires it; record any correction with a corresponding failing-before/passing-after testcase.

### Acceptance criteria and evidence to record

- [ ] A reviewed table gives selected ROM, physical address, internal/external choice and strobes for every slot ROM region and C800–CFFF.
- [ ] Automated tests cover C700/C7FF, C600/C6FF, C800/CFFE/CFFF and changes of selected slot; the C6XX_N compatibility path is preserved.
- [ ] All ROM enables and MEM_ADDR_OUT values are verified at the phase the emulator samples them; any RTL defect is separately explained and tested.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**

Pending. No code has been implemented or tested by creation of this ticket.
