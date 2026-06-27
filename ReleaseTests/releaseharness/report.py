"""Reporting: text matrix to stdout, JSON + HTML to the work area."""

from __future__ import annotations

import json
from pathlib import Path

from .model import RunReport, Status

_GLYPH = {
    Status.PASS: "PASS",
    Status.FAIL: "FAIL",
    Status.SKIP: "skip",
    Status.ERROR: "ERR ",
}


def _checks_order(report: RunReport) -> list[str]:
    seen: list[str] = []
    for r in report.results:
        if r.check not in seen:
            seen.append(r.check)
    return seen


def render_text(report: RunReport) -> str:
    """Render a job x check pass/fail matrix as plain text."""
    jobs: list[str] = []
    for r in report.results:
        if r.job_slug not in jobs:
            jobs.append(r.job_slug)
    checks = _checks_order(report)
    cell: dict[tuple[str, str], Status] = {
        (r.job_slug, r.check): r.status for r in report.results
    }

    lines: list[str] = []
    lines.append(f"Release test report  (mode={'quick' if report.quick else 'full'}, "
                 f"backend={report.backend})")
    lines.append(f"  started {report.started_at}  finished {report.finished_at}")
    lines.append("")

    jobw = max([len("JOB")] + [len(j) for j in jobs]) if jobs else len("JOB")
    for chk in checks:
        header = f"{'JOB':<{jobw}}  {chk}"
        lines.append(header)
        for j in jobs:
            st = cell.get((j, chk))
            glyph = _GLYPH.get(st, "  - ") if st else "  - "
            lines.append(f"  {j:<{jobw}}  {glyph}")
        lines.append("")

    total = len(report.results)
    failed = report.failed
    passed = sum(1 for r in report.results if r.status is Status.PASS)
    skipped = sum(1 for r in report.results if r.status is Status.SKIP)
    lines.append(f"Summary: {passed} pass, {failed} fail/error, {skipped} skip "
                 f"(of {total} checks)")
    return "\n".join(lines)


def write_json(report: RunReport, path: Path) -> None:
    payload = {
        "started_at": report.started_at,
        "finished_at": report.finished_at,
        "quick": report.quick,
        "backend": report.backend,
        "results": [r.as_dict() for r in report.results],
    }
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")


def write_html(report: RunReport, path: Path) -> None:
    jobs: list[str] = []
    for r in report.results:
        if r.job_slug not in jobs:
            jobs.append(r.job_slug)
    checks = _checks_order(report)
    cell = {(r.job_slug, r.check): r.status for r in report.results}
    colors = {Status.PASS: "#cfc", Status.FAIL: "#fcc",
              Status.SKIP: "#eee", Status.ERROR: "#f99"}

    rows = ["<tr><th>job \\ check</th>" + "".join(f"<th>{c}</th>" for c in checks) + "</tr>"]
    for j in jobs:
        tds = [f"<th style='text-align:left'>{j}</th>"]
        for c in checks:
            st = cell.get((j, c))
            label = st.value if st else "-"
            bg = colors.get(st, "#fff") if st else "#fff"
            tds.append(f"<td style='background:{bg}'>{label}</td>")
        rows.append("<tr>" + "".join(tds) + "</tr>")

    html = (
        "<!doctype html><html><head><meta charset='utf-8'>"
        "<title>Geneva release test report</title>"
        "<style>table{border-collapse:collapse}td,th{border:1px solid #999;"
        "padding:4px 8px;font-family:monospace;font-size:13px}</style></head><body>"
        f"<h2>Geneva release test report</h2>"
        f"<p>mode={'quick' if report.quick else 'full'}, backend={report.backend}, "
        f"started {report.started_at}, finished {report.finished_at}</p>"
        "<table>" + "".join(rows) + "</table></body></html>"
    )
    path.write_text(html, encoding="utf-8")


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def render_from_json(path: Path) -> str:
    """Re-render a saved JSON report as text (for the `report` sub-command)."""
    data = load_json(path)
    rep = RunReport(
        started_at=data.get("started_at", ""),
        finished_at=data.get("finished_at", ""),
        quick=data.get("quick", True),
        backend=data.get("backend", ""),
    )
    from .model import CheckResult, Tier
    for r in data.get("results", []):
        rep.add(CheckResult(
            job_slug=r["job"], check=r["check"],
            tier=Tier(r["tier"]), status=Status(r["status"]),
            duration_s=r.get("duration_s", 0.0), message=r.get("message", ""),
            log_path=r.get("log_path"),
        ))
    return render_text(rep)
