# CPLD-HW-0006 — Qualify physical CPLD timing, pin plan and bus isolation

**ID:** `CPLD-HW-0006`
**Status:** `OPEN`
**Title:** Qualify physical CPLD timing, pin plan and bus isolation
**Reporter:** GPT-6 (reviewer finding, following owner-requested HW-0004 completion)
**Reviewer:** Project owner (Kerob Labs); GPT-6
**Assignee:** UNASSIGNED

**Description**

Both MMU and IOU fit the ATF1508ASV-15 PLCC84, including the MMU's six relocated peripherals. However, the accepted fitter outputs are **placement/resource/pin reports**, not a numerical timing-closure report. Before schematic pin locking, PCB manufacture or physical programming sign-off, the design requires documented clock-domain/timing and electrical-bus validation. This is **not a reopen of the accepted software and fitting work** [CPLD-HW-0004](CPLD-HW-0004.md).

**Details**

- Kind: TASK
- Category: `CPLD-HW`
- Milestone: Hardware integration / PCB readiness
- Baseline: `rtl-compile-work` at 2026-11-21, after resolution of HW-0004
- Dependencies: [CPLD-HW-0004](CPLD-HW-0004.md), MMU/IOU intended final physical clocking, external bus transceiver and PCB schematic.
- Source evidence: owner-uploaded fitter files dated 2026-11-21 18:27:18 (MMU) and 18:27:30 (IOU); fitter v1918. They specify `-tpd 15` but **do not report measured maximum frequency, path slack, setup/hold margin or clock-to-output for the final routed board**.
- Both FIT reports end `End fitter, Design FITS` and `$Device PLCC84 fits; JTAG ON; Secure OFF`. These prove fitting, **not all physical timing**.
- Preliminary MMU pin map: `PHI_0` 83, `AN0` 24, `AN1` 25, `AN2` 17, `AN3` 15, `SPK_OUT` 35, `CAS_OUT` 36.
- Preliminary IOU clock map: `PHI_0` 2 and `spi_clk` 83. Physical IOU excludes the MMU's six peripheral outputs.
- JTAG pins for both devices: TDI 14, TMS 23, TCK 62, TDO 71. Keep these accessible for programming.
- This ticket covers **physical timing and interface validation**, not a UART implementation and not a new CPLD feature or software refactor.

**Current Work**

| State | Work / acceptance evidence | Responsible | Next action / reference |
| --- | --- | --- | --- |
| DONE | MMU/IOU fitter and FIT/PIN/TT3 correspondence reviewed; placement, JTAG/security, resource and reported clock-pin mapping pass | Reviewer (GPT-6) | Refer to resolved HW-0004 and source SHA-256 records |
| PENDING | Define actual PHI_0, SPI SCK, reset, address/data valid/hold timing, generated/decoded clocks and inter-domain assumptions | Assignee (UNASSIGNED) | Produce timing budget from relevant device datasheets and board clocks |
| PENDING | Analyse IOU SPI↔PHI_0 transfers and product-term/address-qualified clock paths, including reset and off-phase accesses | Assignee / Reviewer | Timing/CDC report or waveform/measurement with declared frequency and safe margins |
| PENDING | Verify CPU data-bus isolation: IOU DATA OE versus `DATA_EN_N` transceiver control, and ROM/SRAM bus-control contention | Assignee / Reviewer | Schematic bus-direction truth table and timing margin |
| PENDING | Freeze both PLCC84 pin maps, confirm voltage/interface compatibility (3.3 V ASV), reserve JTAG and avoid unconnected NC pins | Assignee / Reviewer | Pin-assignment contract consistent with schematic, fit and board |
| PENDING | Check that reproducible final synth defines, tool versions, current sources/hash and fixed pin assignment yield the expected fit | Assignee | Archive timed FIT/PIN/TT3/JED, source hashes, board version and constraints |
| PENDING | Bench-check physical boards after fabrication (only when available) | Project owner | Hardware timing and function evidence; do not infer bench pass from FIT |

**Notes**

- **2026-11-21 — REVIEWER (GPT-6) — Discovery:** HW-0004 independently reviewed updated owner-uploaded MMU FIT/PIN/TT3 and IOU FIT/PIN/TT3; all 29 direct artifact assertions passed. The MMU uses 50/128 macrocells, 45/64 I/O; IOU 108/128 macrocells, 36/64 I/O. FIT reports only requested 15 ns grade and final fit success, not numeric clock setup/hold or path slack. HW-0004's RTL and resource-fitting result can be RESOLVED while physical timing remains explicitly OPEN here.
- **2026-11-21 — REVIEWER (GPT-6) — Scope distinction:** No physical board/schematic, on-hardware measurement or numeric timing files were supplied. This is a new **hardware readiness** verification workstream; creating it does not imply that the existing RTL or successful fitting is defective.

- **2026-11-21 — REVIEWER (GPT-6) — Ticket registration and verification:** HW-0006 opened by commit `5bd54de` on `rtl-compile-work` to preserve the outstanding numerical timing and PCB electrical checks after HW-0004 fitter sign-off. The owner-supplied FIT/PIN/TT3 artifact checks passed 29/29 (recorded in HW-0004). This new ticket contains acceptance criteria and future verification tasks; it has **no performed physical timing measurements** and must remain OPEN.

**Proposed Change**

Produce a physical sign-off package: device-specific timing budget and clock/control signal requirements, explicit IOU SPI and CPU clock-domain behaviour, bus transceiver/isolation truth table, preserved JTAG, and a frozen pin map matching actual fitted artifacts and board schematic. If a real violation is found, open a focused RTL/hardware correction under the appropriate ticket. Do not optimise or redesign functioning RTL speculatively.

**Acceptance criteria / required evidence**

- [ ] Required SCK/PHI_0 frequency, skew, setup/hold and output-enable timing budgets documented with verifiable sources.
- [ ] SPI/PHI_0 and generated-clock/control interactions reviewed against actual hardware requirements.
- [ ] Final pin assignments, I/O voltages, JTAG, bus transceiver direction/isolation and NC pins checked against schematic.
- [ ] Exact build variants/source hashes and final matched fit/JED set captured; constraints/placement repeatability established.
- [ ] Physical timing and board behaviour confirmed on hardware when available, or explicitly separated into a subsequent commissioning ticket before closure.
- [ ] Explicit project owner and reviewer acceptance recorded; leave OPEN until then.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |
| `rtl-compile-work` | `5bd54de` | UNREVIEWED | Register physical timing, pin-freeze and bus-isolation acceptance separate from HW-0004 |

**Resolution Description**

Pending physical timing and pin-layout qualification.
