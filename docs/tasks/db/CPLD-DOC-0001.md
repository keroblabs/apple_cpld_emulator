# CPLD-DOC-0001 — Add stakeholder attribution and current-work summary to tickets

**ID:** `CPLD-DOC-0001`
**Status:** `REVIEW`
**Title:** Add stakeholder attribution and current-work summary to tickets
**Reporter:** Project owner (Kerob Labs)
**Reviewer:** Project owner; GPT-6
**Assignee:** GPT-6 (methodology and documentation change)

**Description**

The chronological Notes are valuable but long tickets do not show the latest completion state or who made a decision, performed a review, or implemented a change. The project owner requests visible `REPORTER`, `REVIEWER`, and `ASSIGNEE` stakeholders and provenance for historical notes when supported by evidence.

**Details**

- Kind: IMPROVEMENT
- Category: `CPLD-DOC`
- Milestone: Engineering workflow
- Baseline: `rtl-compile-work` at 2026-11-21
- Scope: `docs/tasks/bug-tracking-methodology.md`, relevant touched tickets, index and documentation validation; no change to hardware RTL or runtime.
- Repository Git commits historically use the `Kerob Labs` identity for reviewer-written notes as well as owner activity. Commit author/committer alone is not proof of the stakeholder responsible for the underlying engineering decision.

**Current Work**

| State | Work / acceptance evidence | Responsible | Next action / reference |
| --- | --- | --- | --- |
| DONE | Methodology defines visible ticket stakeholders, editable work summary, actor-labelled notes, and conservative retrospective attribution | GPT-6 | Examine updated methodology and validation result |
| DONE | Add test script validating touched tickets' index/state/role/note requirements | GPT-6 | Run `python3 tests/test_ticket_provenance.py` |
| PENDING | Project owner reviews role model and current-work readability | Project owner | Approve or request rework |
| PENDING | Incremental historical attribution of older tickets as each is touched, without inventing actors | Future assignee | Consult Git history and explicit note text |

**Notes**

- **2026-11-21 — REPORTER (Project owner) — Request:** Make completed versus pending work evident at a glance; identify reporter and reviewer as the project owner and GPT-6, with assignee as the implementing person or agent. Trace existing Notes to their source where reasonably possible.
- **2026-11-21 — REVIEWER (GPT-6) — Investigation:** Existing methodology had append-only date/activity Notes and per-commit tracking but no roles or mutable work summary. Git history identifies commit authors, but historical author `Kerob Labs` does not alone identify the actual reviewer, reporter or assignee.
- **2026-11-21 — ASSIGNEE (GPT-6) — Implementation:** Added three mandatory roles on new/migrated tickets, a short DONE/PENDING/BLOCKED table, prospective role-labelled notes, and a conservative attribution protocol for legacy Notes. Existing note text remains append-only and unmodified.
- **2026-11-21 — ASSIGNEE (GPT-6) — Verification:** Repository-level `tests/test_ticket_provenance.py` document validator added to check the two migrated tickets and index. Additionally, 26/26 direct checks against the committed GitHub document blobs passed: role headers, title/status consistency, current-work state, prospective labelled notes, preservation of original HW-0004 historical Notes, and outstanding post-change fitting criteria. The Python file was not executed on `devel` during this commit; the direct checks are static and not RTL or hardware tests. Owner execution and review remain required.

**Proposed Change**

Add `Reporter`, `Reviewer`, and `Assignee` to the top of new/migrated tickets. Include editable `Current Work` summary, preserve immutable Notes, tag new notes with role and actor, and separately identify the provenance and confidence of any historical attribution. Do not re-label legacy notes without evidence. Index stays six columns; status/title changes propagate in the same commit.

**Acceptance criteria / required evidence**

- [x] Methodology describes roles, summary, actor-labelled notes, historical provenance and uncertainty.
- [x] Touching an existing ticket does not overwrite existing Notes.
- [x] Add validator for matching index/ticket status and roles and prospective note format.
- [ ] Owner reviews and approves ticket readability and actor distinctions.

**Commits**

| Branch | Commit (short hash) | Review Status | Description |
| --- | --- | --- |
| `rtl-compile-work` | `d7cde1b` | UNREVIEWED | Introduce stakeholder/Current Work methodology, provenance validator, and DOC-0001 ticket/index |

**Resolution Description**

Pending owner review; historical provenance remains incremental.
