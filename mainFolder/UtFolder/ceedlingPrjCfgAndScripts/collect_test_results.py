#!/usr/bin/env python3
"""
collect_test_results.py
-----------------------
Scan a directory containing Ceedling test report folders (created by
`unit_test_runner.py` in `TestReportCollection`) and create a CSV file
(`results.csv`) summarizing test results per folder.

Output CSV columns:
- folder: name of the folder containing the test report
- total: total number of tests
- passed: number of passed tests
- failed: number of failed tests

Usage:
    python collect_test_results.py --reports-dir path/to/TestReportCollection --out results.csv

If --reports-dir is omitted, it defaults to `./TestReportCollection`.
"""

from __future__ import annotations

import os
import json
import csv
import argparse
from typing import Optional, Tuple


def find_tests_report_json(folder: str) -> Optional[str]:
    """Return the path to tests_report.json inside folder if present, else None.

    The function checks the folder root and a couple of common subpaths used
    by Ceedling (e.g., `build/artifacts/tests_report.json`).
    """
    candidates = [
        os.path.join(folder, 'tests_report.json'),
        os.path.join(folder, 'build', 'artifacts', 'tests_report.json'),
        os.path.join(folder, 'build', 'tests_report.json'),
    ]
    for p in candidates:
        if os.path.isfile(p):
            return p
    return None


def extract_counts_from_json(path: str) -> Optional[Tuple[int, int, int]]:
    """Read the JSON report and return (total, passed, failed).

    Returns None on parse error or missing Summary.
    """
    try:
        with open(path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        summary = data.get('Summary')
        if not summary:
            return None
        total = int(summary.get('total_tests', 0))
        passed = int(summary.get('passed', 0))
        failures = int(summary.get('failures', 0))
        return total, passed, failures
    except Exception:
        return None


def collect_reports(reports_dir: str) -> list[tuple[str, int, int, int]]:
    """Scan `reports_dir` for subfolders containing test reports and collect counts.

    Returns a list of tuples: (folder_name, total, passed, failed)
    """
    results = []
    if not os.path.isdir(reports_dir):
        raise FileNotFoundError(f"Reports directory not found: {reports_dir}")

    for entry in sorted(os.listdir(reports_dir)):
        entry_path = os.path.join(reports_dir, entry)
        if not os.path.isdir(entry_path):
            continue
        json_path = find_tests_report_json(entry_path)
        if json_path:
            counts = extract_counts_from_json(json_path)
            if counts:
                total, passed, failed = counts
            else:
                total = passed = failed = 0
        else:
            total = passed = failed = 0
        results.append((entry, total, passed, failed))

    return results


def write_csv(rows: list[tuple[str, int, int, int]], out_path: str) -> None:
    """Write the collected rows to a CSV file (with header)."""
    with open(out_path, 'w', encoding='utf-8', newline='') as fh:
        writer = csv.writer(fh)
        writer.writerow(['folder', 'total', 'passed', 'failed'])
        for row in rows:
            writer.writerow(row)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description='Collect Ceedling test report counts into CSV')
    p.add_argument('--reports-dir', default='TestReportCollection', help='Directory containing test report folders')
    p.add_argument('--out', default='TestReportCollection/results.csv', help='Output CSV file path')
    return p.parse_args()


def main() -> None:
    args = parse_args()
    rows = collect_reports(args.reports_dir)
    write_csv(rows, args.out)
    print(f"Wrote {len(rows)} rows to {args.out}")


if __name__ == '__main__':
    main()
