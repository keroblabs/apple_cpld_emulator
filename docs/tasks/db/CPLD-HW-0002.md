# CPLD-HW-0002 — Add shared BUS_RD_N/BUS_WR_N controls to RTL and emulator

**ID:** `CPLD-HW-0002`  
**Status:** `REVIEW`
**Title:** Add shared BUS_RD_N/BUS_WR_N controls to RTL and emulator

**Description**

The MMU already has **separate SRAM and flash select/decode signals**. Do not waste four pins on per-device read/write strobes: implement and expose **exactly two new shared active-low bus controls**, `BUS_RD_N` and `BUS_WR_N`. Each memory device interprets these in combination with its **existing selection signal**, so that the two shared strobes save two CPLD pins. Propagate the new RTL outputs through the Verilator adapter and make the emulator use the *actual* strobe/select combination when accessing RAM and parallel flash. Preserve the ability to use the NOR flash's write/update capability; do **not** permanently disable flash writes.

This is an **RTL and emulator software implementation ticket**. Hardware layout/schematic/PCB design, voltage translation and physical chip bring-up are out of scope. Nevertheless, the RTL **must synthesise and fit the intended ATF1508ASV**, including package I/O allocation, macrocells and timing.

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: M1 — ROM mapping / MMU hardware interface
- Baseline: `main` inspected 2026-11-21; documentation updated on `cpld-hw-0001-0003-tickets`. Implementation branch per owner/developer workflow.
- Dependencies: [CPLD-HW-0001](CPLD-HW-0001.md) (device, SPI ownership and 128 KiB decisions), [CPLD-ROM-0005](CPLD-ROM-0005.md) (immutable *image-layout contract*, not immutable physical NOR).
- Scope: MMU RTL, combined Verilator top, C++ RTL interface, emulator RAM/flash selection and tests, synthesis/fitting report. No unrelated protocol, firmware or PCB redesign.

**Notes**

- **2026-11-21 — Source observation:** `rtl_MMU/MMU.v`, `rtl_MMU/MMU_addr.v` and `rtl_IOU_MMU_TB/IOU_MMU_tb.v` currently expose `RAM_EN_N` and `ROM_EN_N`; the existing card-ROM strobes are `IOSEL_N`, `IOSTB_N` and `C6XX_N`. These are **not interchangeable**: `ROM_EN_N` alone does not select every flash-owned card ROM read, and `ROM_EN_N` is currently read-qualified. Verify the *actual* select policy, including optional external slot 6, before coding.
- **2026-11-21 — Emulator observation:** `cpld/rtl_interconnect.cpp` captures the existing selects; `emulator/onboard/memory_controller.c` uses the caller's read/write direction and those selects to choose a backing. New hardware strobes must become observable and functionally consumed, not just asserted in unit tests.
- **2026-11-21 — Owner correction (supersedes initial four-output proposal):** Two shared outputs **`BUS_RD_N` and `BUS_WR_N` only**; retain current independent SRAM/flash chip selects. This saves two external CPLD pins and reduces duplicated decode.
- **2026-11-21 — Owner correction (flash updates):** Preserve/exploit the **SST39VF010-70-4I-NHE in-system flash write/update capability**; do not specify a permanent WE# tie-high or always-inactive `FLASH_WR_N`. A write strobe by itself does not program NOR: the flash must be legitimately selected and receive the valid command/address/data sequence. Establish and test the authorised route without introducing accidental writes or silently creating a new Apple-visible API.
- **2026-11-21 — Scope boundary:** **Software only** (Verilog RTL, emulator and verification). Fitting to the real ATF1508ASV target is required evidence for software readiness; physical board design, measurements, power sequencing and programmer readback are separate work.
- **2026-11-21 — Rework:** Added exactly `BUS_RD_N` and `BUS_WR_N` through `MMU_addr`, `MMU`, the combined Verilator top, adapter capture and emulator memory selection. Both strobes are inactive during reset, PHI_0 low and DMA-owned phases. Reads use retained RAM/ROM/card selects, including IOSEL, IOSTB and slot 6. Writes are currently enabled only for selected RAM; ordinary CPU writes to ROM ranges leave both memory strobes inactive.
- **2026-11-21 — Verification:** `bash rtl_MMU/verification/run_tests.sh` passes 787 checks / 0 failures in both default and `FULL_MMU_ADDR` builds. `bash emulator/onboard/verification/run_tests.sh` passes 29,995 combined ROM-bus checks / 0 failures in each address build; exported-image SHA-256 is identical between the builds. Tests cover RTL-strobe capture, RAM read/write, all flash-backed ROM selections, CFFF release, I/O, reset/idle, DMA, turnaround, mutually exclusive outputs and direction-mismatch probes proving the emulator consumes the sampled strobe/select pair.
- **2026-11-21 — Flash-update gap:** The source has no authorised flash-write grant or command-selection route. `ROM_EN_N` is read-qualified, while card-ROM selects do not authorize NOR programming on writes. `BUS_WR_N` is therefore not asserted for flash and the emulator has no mutable NOR command model. No flash update was tested. Owner review is required to define the minimum authorised update context and command path within the ticket's no-new-pin/no-new-Apple-API constraints; preserve the existing flash write capability as an unresolved dependency.
- **2026-11-21 — Fitter evidence:** `yosys`, `atf15xx_yosys` and `fit1508` are unavailable in this environment, and no fitter script was found under `/home/marcio/Tools`. The checked-in `rtl_MMU/MMU.fit` and `rtl_IOU/IOU.fit` are historical baseline reports from November 2025, not results for this revision. ATF1508ASV macrocells, pin allocation and timing remain unverified; keep this ticket in REVIEW pending the actual fitter run and flash-update design review.
- **2026-11-21 — Owner-requested build and verification retest:** From `rtl_IOU_MMU_TB/`, `./build.sh IOU_MMU_tb.v` freshly rebuilt the 18-module Verilator model. `cmake --build build --clean-first --parallel 2` then rebuilt and linked `apple_cpld` successfully (100%). `bash rtl_MMU/verification/run_tests.sh` passed 787 checks / 0 failures in both default and `FULL_MMU_ADDR` builds. `bash emulator/onboard/verification/run_tests.sh` passed 33 ROM-backing checks and 30,010 combined ROM-bus checks / 0 failures in each address build; exported images matched. `bash rtl_IOU/verification/run_spi_tests.sh` passed 59 checks / 0 failures.

- **2026-11-21 — Independent review (GPT-6):** Inspected commit `2fbb9eb` and the current branch: `MMU_addr.v` qualifies the two shared strobes with reset released, DMA ownership, PHI_0 high, direction and existing selects; `MMU.v` and the combined Verilator top expose exactly those outputs. The C++ capture and `memory_controller.c` consume the sampled bus strobe plus select, and the changed ROM-bus tests explicitly probe mismatched caller direction. The SPI interface and existing memory map are untouched by this ticket. **The implemented bus-strobe code is APPROVED; no code changes requested.** Owner reports manual PR#3/PR#0 success, and developer records repeated Verilator build, 787 MMU checks (both builds), 30,010 ROM-bus checks per build and preserved 128 KiB image hash; reviewer inspected sources and test evidence but did **not independently execute** the test suite. **Ticket remains REVIEW** because no revised ATF1508ASV synthesis/fitter/timing evidence exists and the parallel NOR update route is only a documented gap: current `BUS_WR_N` is RAM-only, `ROM_EN_N` is read-qualified, and the emulator intentionally has no flash-command model. The SST39VF010's physical programmability must not be confused with a working CPLD programming interface. Resolve the update-path scope with the owner, document/test the authorised flash select/grant or explicitly create an approved follow-on, and attach a current fitter report before resolving this ticket. No PCB design is required here.

**Proposed Change**

### 1. Existing selects plus common bus strobes

Export these **two new outputs** from MMU RTL and the combined Verilator top:

| Signal | Active level | Intended use |
| --- | --- | --- |
| `BUS_RD_N` | 0 | Valid granted memory-bus **read** phase |
| `BUS_WR_N` | 0 | Valid granted memory-bus **write** phase |

**Use the existing memory selects** (not extra new RD/WR pins). Logical device actions are:

| Existing target selection | `BUS_RD_N` | `BUS_WR_N` | Action |
| --- | ---: | ---: | --- |
| SRAM selected | 0 | 1 | SRAM read |
| SRAM selected | 1 | 0 | SRAM write |
| Flash selected for an owned read | 0 | 1 | Parallel flash read |
| Flash selected for an **authorised flash-update cycle** | 1 | 0 | Parallel flash program/erase **command write** |
| Neither memory selected, or I/O/external-card owner | 1 | 1 or phase-qualified inactive | No SRAM/flash access |
| Reset/idle/invalid cycle | 1 | 1 | No memory operation |

The above is a **logical contract**, not a claim that the existing `ROM_EN_N` grants flash writes or is a complete physical chip-select. Review the actual decoded flash ownership (internal Apple Cx, card IOSEL/IOSTB, slot-6 conflict, `MODE_SEL`, `$CFFF`) separately from `CPU_RW`. Keep CE/select and RD/WR roles separate. Document explicitly the truth table for phase-qualified idle/IO cycles, DMA and mode changes. Both shared strobes must **never** assert together.

### 2. RTL implementation

1. Generate `BUS_RD_N` and `BUS_WR_N` in `rtl_MMU/MMU_addr.v` or a tightly scoped decode module; expose in `rtl_MMU/MMU.v` and `rtl_IOU_MMU_TB/IOU_MMU_tb.v`. Make reset/idle safe and verify PHI_0 phase and data/address setup/hold.
2. Preserve existing `RAM_EN_N`, `ROM_EN_N`, `IOSEL_N`, `IOSTB_N`, `C6XX_N`, translated memory address, DMA behaviour and optional external slot 6. Do **not** redefine selection behaviour simply to make the new signals easy.
3. **Flash programming is a required capability, not automatically enabled by CPU writes.** Verify whether any *current* firmware/RTL path can legitimately assert flash selection during a programming sequence; `ROM_EN_N` is presently read-qualified. Provide a source-level description and a failing regression for any missing select/grant. Ensure the shared `BUS_WR_N` reaches flash only when its existing select is valid in a controlled update context; **ordinary CPU writes to ROM ranges must not be mistaken for authorised flash commands**. If an authorisation/grant or command path must be added, submit the minimum concrete design/gap for owner review rather than assuming new control registers, changes to the IOMEM/SPI protocol or a new dedicated pin.
4. Keep the 128 KiB image/map unchanged. Flash A16 is low for current CPU ROM mapping and the upper 64 KiB remains reserved: do **not** quietly claim in-system update access to the upper bank without a demonstrated address route and separate approval.

### 3. Emulator integration and tests

1. Capture **both actual Verilated outputs** in `cpld/rtl_model.h` and `cpld/rtl_interconnect.cpp`, with reset defaults; update `cpld/rtl_memory_bus.cpp` and `emulator/onboard/memory_controller.c` as appropriate. RAM reads/writes and flash reads must be determined from **existing select + `BUS_RD_N`/`BUS_WR_N`**, not solely from the call's `is_read` value. CPU direction remains an input to drive RTL, but cannot substitute for inspecting its resulting strobes.
2. Keep the normal ROM backing/export invariant: the emulator starts from the compiled 131072-byte image; ordinary reads continue to return identical bytes. If flash update sequencing is modelled, separate **mutable runtime flash state** from the compiled initial image and explicitly define how update commands affect it; do not silently change the `-w` **startup-image export** contract.
3. Test SRAM read/write, internal/slot flash read, external slot6/IOSEL/IOSTB ownership, `$CFFF` release, CPU write to a ROM-mapped address without authorisation, valid authorised flash command-write strobe, read→write turnaround, I/O, reset, DMA and invalid/aborted cycles in both normal and `FULL_MMU_ADDR` builds.
4. For flash updates, separately verify **write eligibility/selection** and the flash-device command sequence; a toggling `BUS_WR_N` by itself is **not** proof of flash byte programming/sector erase. If the update path cannot yet be fully exercised, record the precise missing software functionality and make it a separate reviewed dependency; do not mark the update capability as verified.

### 4. CPLD implementation feasibility (without designing the board)

Run the available Yosys/`atf15xx_yosys` and fitter flow for the **actual intended ATF1508ASV variant**. Compare two additional I/O pins against the previous four-output proposal, report final macrocell/logic use, pin assignments, timing (particularly shared read/write strobe glitches and mutually exclusive phases), and any conflict with fixed I/O/clock limitations. RTL fit is a gate; **PCB schematic, level shifters and physical signals remain outside this ticket**. On fit failure, provide report and targeted RTL recommendation for owner review.

**Acceptance criteria / evidence**

- [x] Exactly **two** new RTL pins (`BUS_RD_N`, `BUS_WR_N`) appear in both Verilator variants; no `SRAM_*_N` or `FLASH_*_N` per-device strobe pins are added.
- [x] Existing SRAM/flash selects retained; truth table and qualifiers cover both directions, flash-owned card ROM cases and optional external slot6 correctly.
- [ ] `BUS_RD_N` and `BUS_WR_N` are never active simultaneously; reset, idle and bus transitions are deterministic and free of unintended pulses.
- [ ] Emulator consumes the RTL outputs with the existing selects and passes full Apple IIe, RAM switching, slot-ROM, IOMEM/keyboard and Disk II regressions.
- [ ] Flash write/update capability is **not designed out**; a valid flash-selection/program-command path is documented and tested where implemented, or the missing path is explicitly captured as a blocker/follow-on (never misreported as implemented).
- [x] Default ROM backing and the 131072-byte `-w` export remain byte-identical before any explicit runtime update; upper bank policy unchanged.
- [ ] ATF1508ASV synthesis/fitter, external pin usage and timing evidence attached. Physical PCB fabrication and electrical tests are not acceptance criteria.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `cpld-hw-0001-0003-tickets` | `2fbb9eb` | `APPROVED` | Add shared bus strobes, capture and consume them in the emulator; verify both MMU address builds and combined ROM-bus regressions. |

**Resolution Description**

Independent code review **APPROVES commit `2fbb9eb`**, supported by developer regressions and owner manual PR#3/PR#0. The ticket remains **REVIEW** pending current ATF1508ASV synthesis/fitter/resource/timing evidence and owner disposition of the currently missing authorised NOR flash-update route. Full physical timing and board work are separate. No physical hardware design or in-system flash-programming protocol change is included.
