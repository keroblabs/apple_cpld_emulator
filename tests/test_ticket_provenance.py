#!/usr/bin/env python3
"""Validate the new ticket ownership/summary conventions on migrated tickets."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
method = (root / "docs/tasks/bug-tracking-methodology.md").read_text()
index = (root / "docs/tasks/task_list.md").read_text()
ids = ["CPLD-DOC-0001", "CPLD-HW-0004"]
checks = 0

def require(ok, explanation):
    global checks
    if not ok:
        raise AssertionError(explanation)
    checks += 1
    print("PASS", explanation)

require("## 4.1 Historical note attribution and provenance" in method, "methodology documents historical provenance")
require("**Current Work**" in method and "**Reporter:**" in method, "methodology specifies roles and current-work table")
require("UNKNOWN" in method and "committer" in method, "uncertain historical authorship is explicit")
require("CLOSED only after the project owner merges" in method and "into `main`" in method, "closure requires main merge")

for issue_id in ids:
    body = (root / f"docs/tasks/db/{issue_id}.md").read_text()
    def field(k):
        m = re.search(r"^\*\*" + re.escape(k) + r":\*\*\s*(.+)$", body, re.M)
        if not m:
            raise AssertionError(f"{issue_id}: missing {k}")
        return m.group(1).strip().strip('`')
    for role in ("Reporter", "Reviewer", "Assignee"):
        require(bool(field(role)), f"{issue_id}: {role} populated")
    require("**Current Work**" in body and "| DONE |" in body and "| PENDING |" in body, f"{issue_id}: done and pending work visible")
    require("**Notes**" in body and any(line.startswith("- **2026-11-21 — ") and any(f"— {role} (" in line for role in ("REPORTER", "REVIEWER", "ASSIGNEE")) for line in body.splitlines()), f"{issue_id}: prospective role-labelled note exists")
    rows = [line for line in index.splitlines() if line.startswith(f"| `{issue_id}` |")]
    require(len(rows) == 1, f"{issue_id}: exactly one index row")
    cols = [cell.strip() for cell in rows[0].strip().strip("|").split("|")]
    require(len(cols) == 6 and cols[2] == field("Status") and cols[3] == field("Title"), f"{issue_id}: index status/title match")

require("**Status:** `CLOSED`" not in (root / "docs/tasks/db/CPLD-HW-0004.md").read_text(), "CPLD-HW-0004 is not CLOSED before integration")
print(f"All {checks} ticket methodology checks passed.")
