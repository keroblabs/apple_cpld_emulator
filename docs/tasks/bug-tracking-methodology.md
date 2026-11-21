# Apple CPLD Emulator — Task and Bug Tracking Methodology

**Status:** Working project process (established 2026-11-20)  
**Scope:** `keroblabs/apple_cpld_emulator` hardware RTL, firmware, STM32 controller interfaces, IOMEM, emulator integrations, tests and documentation.  
**Provenance:** Adapted for this repository from the existing [root task methodology](../bug-tracking-methodology.md), which is a Tima/Plus-derived document and is *not* automatically the Apple CPLD issue tracker.

## 1. Governing principles

- **One issue, one file:** Every ticket lives at `docs/tasks/db/<ID>.md`, irrespective of length. The only index is `docs/tasks/task_list.md`.
- **Review before change:** Opening tickets and describing a candidate design does not authorise implementation or a merge. Investigation and design approval are explicit.
- **Traceability:** Document source evidence, unknowns, dependencies, acceptance tests, decisions, commits and resolution per ticket.
- **Visible responsibility:** Every new/migrated ticket names its reporter, reviewer(s), and current assignee (or `UNASSIGNED`). Every new note identifies the actor and their role; ownership of *recording* a note is not automatically ownership of its *decision*.
- **Current work at a glance:** Each new/migrated ticket maintains a concise, editable `Current Work` summary of DONE, PENDING and BLOCKED items, with clear evidence and next action. The chronological Notes remain the immutable decision history.
- **Preserve existing functionality:** No unreviewed change to the 65816, keyboard, slot6 floppy, Apple C0Fx mailbox or unrelated C video implementation. RTL changes require failing tests and approved scope.
- **Implementers commit locally:** Developers hand off local commits/SHAs and test results without pushing, making PRs or merging; the project owner controls publication after review. Owner-directed tracker creation or maintenance is not a developer implementation handoff.
- **No silent closure:** Tests and reviewer acceptance precede RESOLVED. For this project, a reviewed change is **CLOSED only after the project owner merges its approved PR/changes into `main`**, or approves an explicit no-change disposition. Merely committing on `rtl-compile-work` does not close a ticket.

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

Statuses: `OPEN` (awaiting assessment/approval), `IN_PROGRESS` (agreed work active), `REVIEW` (change/evidence presented), `RESOLVED` (reviewed and accepted; awaiting owner-controlled integration), `CLOSED` (merged into `main` or owner-approved no-change disposition), `DEFERRED` (reason and re-entry condition documented). Common sequence: OPEN → IN_PROGRESS → REVIEW → RESOLVED → CLOSED. Reopening requires a dated explanation and new status.

Kinds: `BUG` (confirmed or suspected failure; label unverified hypotheses), `TASK` (bounded design or implementation), `IMPROVEMENT` (non-mandatory refinement). A ticket's initial kind and status appear identically in the index and ticket file.

## 4. Index and ticket format

The index groups tickets by category and has six columns, exactly: **ID, Kind, Status, Title, Milestone, Ticket**, with each ticket linked as `db/<ID>.md`. Sorted permanent IDs within each category. Do not place full technical descriptions in the index.

Each individual ticket has exactly one issue. New and migrated tickets have the required fields in this order: **ID, Status, Title, Reporter, Reviewer, Assignee, Description, Details, Current Work, Notes, Proposed Change, Commits, Resolution Description**. Older ticket files may retain the legacy layout until incrementally migrated; never rewrite old entries merely to satisfy the new format.

Role definitions: **Reporter** is the person/agent who raised or requested the issue, **Reviewer** is the explicitly designated person/agent evaluating evidence and acceptance (may include the project owner and GPT-6), and **Assignee** is the person/agent currently responsible for implementing or verifying the work. Use actual identities or stable role labels; put `UNASSIGNED` or `UNKNOWN` when the evidence does not identify someone. Reporter/reviewer can be plural; assignee changes require a dated note. One person can hold multiple roles but they must be stated separately.

Example ticket template:

```markdown
# CPLD-AREA-0001 — Short title

**ID:** `CPLD-AREA-0001`
**Status:** `OPEN`
**Title:** Short title
**Reporter:** Project owner (Kerob Labs)
**Reviewer:** Project owner; GPT-6
**Assignee:** UNASSIGNED

**Description**
Evidence-based gap and why it matters.

**Details**
- Kind: TASK
- Category: CPLD-AREA
- Milestone: M1 — ...
- Baseline: `rtl-rework` at the date of creation
- Dependencies: ...
- Scope and evidence: ...

**Current Work**

| State | Work / acceptance evidence | Responsible | Next action / reference |
| --- | --- | --- | --- |
| DONE | Verified baseline or completed result | Reviewer | Link to commit/test |
| PENDING | Work not yet verified | Assignee (or UNASSIGNED) | Exact next test |
| BLOCKED | Unresolved dependency, if any | Named actor | Clear unblock condition |

**Notes**
- **YYYY-MM-DD — REPORTER (Project owner) — Request:** Why the work was requested.
- **YYYY-MM-DD — REVIEWER (GPT-6) — Analysis:** Observation, evidence and assessment.
- **YYYY-MM-DD — ASSIGNEE (Developer) — Test:** Exact test command and result.

**Proposed Change**
A candidate approach, explicit constraints and acceptance criteria.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- | --- |

**Resolution Description**
Pending.
```

Preserve dated Notes as an **append-only** decision/rework log. Update the index in the same commit whenever status/title/kind changes. Keep superseded implementation commit records visible. `Current Work` is a **mutable summary**, not a second log: update it whenever a new evidence note changes DONE/PENDING/BLOCKED status, keeping links into Notes, commits, tests, or review findings. Never mark `DONE` from an unexecuted test or a proposed change.

## 4.1 Historical note attribution and provenance

- **Prospective notes:** Every new Notes entry is `**YYYY-MM-DD — ROLE (actor) — Activity:** ...`, where ROLE is `REPORTER`, `REVIEWER` or `ASSIGNEE`. Attribute actions to the actor who performed them; if a reviewer writes up the owner's test report, label the writer `REVIEWER` and state `owner-reported, not independently executed` in its content.
- **Request versus recording:** A note may be authored by a reviewer while describing a reporter request. State both explicitly, e.g. `**... — REVIEWER (GPT-6) — Recorded request:** Project owner requested ...`. Never confuse Git *committer* with the stakeholder who requested, approved or tested the change.
- **Retrospective attribution:** Before assigning actors to legacy notes, inspect the matching `git log --follow -- <ticket-path>` and relevant commit diffs (`git show <sha> -- <ticket-path>` or `git blame`). Record `Git author/committer`, the commit SHA, and any **explicit** in-note actor evidence separately. Git account attribution identifies who committed the text, **not necessarily who actually requested, reviewed or implemented it**; this repo contains commits under a shared Kerob Labs author identity.
- **No invention:** If a legacy note cannot be reliably assigned to a person/agent, preserve its exact text and mark attribution `UNKNOWN` in a separate provenance table under Details, with confidence and evidence. Do not silently re-label or rewrite append-only historical notes, or use a generic Git author as proof that a person performed a test. Progressively annotate touched tickets only; no all-ticket backfill is implied.
- **Role handoffs:** Note any `Assignee` or `Reviewer` change with date, changing actor, original actor, new actor and evidence. A prospective test must have executor and a pass/fail/not-run outcome, even when the person writing the entry differs.


## 5. Review/implementation rules

1. **Discover/triage** and record source behaviour separately from assumptions. For MMU/ROM logic, simulate realistic PHI_0 sequences and mark suspected issues *unverified* until reproduced.
2. **Agree protocol decisions** before locking byte layout, ROM mapping, hardware choice or firmware calling convention. Preserve established Apple-facing C0Fx and ProDOS contracts.
3. **Implement on the approved branch** and commit focused changes locally; record SHA, changed files and tests. Developers do not push/merge under the agreed workflow.
4. **Verify** with Verilator tests, ROM dump/slot probes, byte-exact IOMEM transfer and actual ProDOS boot where relevant. A build-only result is not sufficient.
5. **Review and record** review findings, with an explicit reviewer identity, the specific actor who executed each test, and the Git commit/event when traceable. Commit statuses are `UNREVIEWED`, `IN_REVIEW`, `CHANGES_REQUESTED`, `APPROVED`, or `SUPERSEDED`.
6. **Resolve only after approval** with exact tested environment and results; **close when the owner merges to `main`**, or records an accepted no-change disposition. A working-branch commit, fitter success or PR alone is not CLOSED.

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
