# Apple CPLD Emulator — Task and Bug Tracking Methodology

**Status:** Working project process (established 2026-11-20)  
**Scope:** `keroblabs/apple_cpld_emulator` hardware RTL, firmware, STM32 controller interfaces, IOMEM, emulator integrations, tests and documentation.  
**Provenance:** Adapted for this repository from the existing [root task methodology](../bug-tracking-methodology.md), which is a Tima/Plus-derived document and is *not* automatically the Apple CPLD issue tracker.

## 1. Governing principles

- **One issue, one file:** Every ticket lives at `docs/tasks/db/<ID>.md`, irrespective of length. The only index is `docs/tasks/task_list.md`.
- **Review before change:** Opening tickets and describing a candidate design does not authorise implementation or a merge. Investigation and design approval are explicit.
- **Traceability:** Document source evidence, unknowns, dependencies, acceptance tests, decisions, commits and resolution per ticket.
- **Preserve existing functionality:** No unreviewed change to the 65816, keyboard, slot6 floppy, Apple C0Fx mailbox or unrelated C video implementation. RTL changes require failing tests and approved scope.
- **Implementers commit locally:** Developers hand off local commits/SHAs and test results without pushing, making PRs or merging; the project owner controls publication after review. Owner-directed tracker creation or maintenance is not a developer implementation handoff.
- **No silent closure:** Tests and reviewer acceptance precede RESOLVED; integration precedes CLOSED.

## 2. Paths, IDs and categories

```text
docs/tasks/
  bug-tracking-methodology.md
  task_list.md
  db/
    CPLD-ROM-0001.md
    CPLD-<AREA>-NNNN.md
```

Permanent IDs use `CPLD-<AREA>-NNNN`. Numbers increment within a category and are never reused. Categories for the current hard-disk programme: `ROM` (decoder, flash layout and backing), `HW` (STM32/CPLD physical topology), `FW` (Apple-side ProDOS firmware), `IOM` (IOMEM commands and SPI reliability), `HDD` (block service and disk image) and `TEST` (integrated verification). New areas may be added without renumbering existing tickets.

Milestones for this programme: **M1 — ROM mapping**, **M2 — Slot 7 firmware**, **M3 — IOMEM disk service**, **M4 — Hard-disk integration**. Milestones indicate workstreams; explicit ticket dependencies are authoritative. The project roadmap may later incorporate these milestones.

## 3. States and kinds

Statuses: `OPEN` (awaiting assessment/approval), `IN_PROGRESS` (agreed work active), `REVIEW` (change/evidence presented), `RESOLVED` (reviewed and accepted), `CLOSED` (integrated or disposition completed), `DEFERRED` (reason and re-entry condition documented). Common sequence: OPEN → IN_PROGRESS → REVIEW → RESOLVED → CLOSED. Reopening requires a dated explanation and new status.

Kinds: `BUG` (confirmed or suspected failure; label unverified hypotheses), `TASK` (bounded design or implementation), `IMPROVEMENT` (non-mandatory refinement). A ticket's initial kind and status appear identically in the index and ticket file.

## 4. Index and ticket format

The index groups tickets by category and has six columns, exactly: **ID, Kind, Status, Title, Milestone, Ticket**, with each ticket linked as `db/<ID>.md`. Sorted permanent IDs within each category. Do not place full technical descriptions in the index.

Each individual ticket has exactly one issue, and fields in this order:

```markdown
# CPLD-AREA-0001 — Short title

**ID:** `CPLD-AREA-0001`
**Status:** `OPEN`
**Title:** Short title

**Description**
Evidence-based gap and why it matters.

**Details**
- Kind: TASK
- Category: CPLD-AREA
- Milestone: M1 — ...
- Baseline: `rtl-rework` at the date of creation
- Dependencies: ...
- Scope and evidence: ...

**Notes**
- **YYYY-MM-DD — Discovery:** Evidence and rationale.

**Proposed Change**
A candidate approach, explicit constraints and acceptance criteria.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**
Pending.
```

Preserve dated Notes as an **append-only** decision/rework log. Update the index in the same commit whenever status/title/kind changes. Keep superseded implementation commit records visible.

## 5. Review/implementation rules

1. **Discover/triage** and record source behaviour separately from assumptions. For MMU/ROM logic, simulate realistic PHI_0 sequences and mark suspected issues *unverified* until reproduced.
2. **Agree protocol decisions** before locking byte layout, ROM mapping, hardware choice or firmware calling convention. Preserve established Apple-facing C0Fx and ProDOS contracts.
3. **Implement on the approved branch** and commit focused changes locally; record SHA, changed files and tests. Developers do not push/merge under the agreed workflow.
4. **Verify** with Verilator tests, ROM dump/slot probes, byte-exact IOMEM transfer and actual ProDOS boot where relevant. A build-only result is not sufficient.
5. **Review and record** review findings. Commit statuses are `UNREVIEWED`, `IN_REVIEW`, `CHANGES_REQUESTED`, `APPROVED`, or `SUPERSEDED`.
6. **Resolve only after approval** with exact tested environment and results; **close on integration** (or accepted no-change disposition).

## 6. Compatibility and programme constraints

- Preserve `C6XX_N` behaviour so a real floppy controller remains optional. Use `ROM_EN_N` + `MEM_ADDR_OUT` as the preferred ROM backing-selection interface once validated.
- **STM32 is the planned SPI master** and owns RTC and SPI flash; **do not add a CPLD SPI-master implementation** to this milestone.
- Distinguish storing ROM images in serial flash from driving the CPU's data bus at ROM-read timing; the hardware ROM access architecture must be resolved before assuming the physical design will work.
- Slot7 ProDOS firmware uses the Apple-side IOMEM mailbox and a **16-bit** block index, 512-byte sectors. The older 32-bit LBA example is only a proposal and must be updated when the 16-bit contract is approved.
- Test the emulator first; hardware and STM32 firmware must have their own evidence before being declared complete.

## 7. Technical references

- [Existing SPI/IOMEM protocol](../spi-iomem-protocol.md)
- [Existing 32-bit LBA example requiring update](../iomem-command-implementation-reference.md)
- [RTL scheduler architecture](../rtl-rework-architecture.md)
- [Apple ProDOS 8 block-driver interface](https://prodos8.com/docs/techref/adding-routines-to-prodos/)
- [ProDOS Technical Note 20 — SmartPort and mirrored devices](https://prodos8.com/docs/technote/20/)
- [ProDOS Technical Note 21 — device identification](https://prodos8.com/docs/technote/21/)
