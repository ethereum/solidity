#!/usr/bin/env python3
"""Summarizes gas differences from semantic test diff.

The script collects all gas differences present in git diff of semantic tests
and summarizes them in the form of a table that's ready to post in a GitHub comment.
Only changes that are already committed are taken into account.
Changes that are only staged or not staged or committed at all are ignored.

Useful for reviewing the gas impact of a specific PR / branch.
Instead of tediously going through each individual change, it's recommended to review the table.

Dependencies: parsec, tabulate
"""

import subprocess
import sys
from argparse import ArgumentParser, RawDescriptionHelpFormatter
from pathlib import Path
from enum import Enum
from parsec import generate, ParseError, regex, string, optional
from tabulate import tabulate

class Kind(Enum):
    Ir = 1
    IrOptimized = 2
    Legacy = 3
    LegacyOptimized = 4

class Diff(Enum):
    Minus = 1
    Plus = 2

SEMANTIC_TEST_DIR = Path("test/libsolidity/semanticTests/")

minus = string("-").result(Diff.Minus)
plus = string("+").result(Diff.Plus)

space = string(" ")
comment = string("//")
colon = string(":")

gas_ir = string("gas ir").result(Kind.Ir)
gas_ir_optimized = string("gas irOptimized").result(Kind.IrOptimized)
gas_legacy_optimized = string("gas legacyOptimized").result(Kind.LegacyOptimized)
gas_legacy = string("gas legacy").result(Kind.Legacy)
code_suffix = string("code")

def number() -> int:
    """Parse number."""
    return regex(r"([0-9]*)").parsecmap(int)

@generate
def diff_string() -> (Kind, Diff, int):
    """Usage: diff_string.parse(string)

    Example string:

    -// gas irOptimized: 138070

    """
    diff_kind = yield minus | plus
    yield comment
    yield space
    codegen_kind = yield gas_ir_optimized ^ gas_ir ^ gas_legacy_optimized ^ gas_legacy
    yield optional(space)
    yield optional(code_suffix)
    yield colon
    yield space
    val = yield number()
    return (diff_kind, codegen_kind, val)

def collect_statistics(lines) -> (int, int, int, int, int, int):
    """Returns

    (old_ir_optimized, old_legacy_optimized, old_legacy, new_ir_optimized,
    new_legacy_optimized, new_legacy)

    All the values in the same file (in the diff) are summed up.

    """
    if not lines:
        raise RuntimeError("Empty list")

    out = [
        parsed
        for line in lines
        if line.startswith('+// gas ') or line.startswith('-// gas ')
        if (parsed := diff_string.parse(line)) is not None
    ]
    diff_kinds = [Diff.Minus, Diff.Plus]
    codegen_kinds = [Kind.IrOptimized, Kind.LegacyOptimized, Kind.Legacy]
    return tuple(
        sum(
            val
            for (diff_kind, codegen_kind, val) in out
            if diff_kind == _diff_kind and codegen_kind == _codegen_kind
        )
        for _diff_kind in diff_kinds
        for _codegen_kind in codegen_kinds
    )

def semantictest_statistics(base_branch: str):
    """Prints the tabulated statistics that can be pasted in github."""
    def parse_git_diff(fname):
        diff_output = subprocess.check_output(
            ["git", "diff", "--unified=0", base_branch, "HEAD", fname],
            universal_newlines=True
        ).splitlines()
        if len(diff_output) == 0:
            return None
        return collect_statistics(diff_output)

    def percent(old, new):
        return (int(new) - int(old)) / int(old) * 100 if int(old) != 0 else None

    def percent_or_zero(old, new):
        result = percent(old, new)
        return result if result is not None else 0

    def format_percent(percentage):
        if percentage is None:
            return ''
        prefix = (
            # Distinguish actual zero from very small differences
            '+' if round(percentage) == 0 and percentage > 0 else
            '-' if round(percentage) == 0 and percentage < 0 else
            ''
        )
        return f'{prefix}{round(percentage)}%'

    def stat(old, new):
        return format_percent(percent(old, new))

    table = []

    if not SEMANTIC_TEST_DIR.is_dir():
        sys.exit(f"Semantic tests not found. '{SEMANTIC_TEST_DIR.absolute()}' is missing or not a directory.")

    for path in SEMANTIC_TEST_DIR.rglob("*.sol"):
        fname = path.as_posix()
        parsed = parse_git_diff(fname)
        if parsed is None:
            continue
        assert len(parsed) == 6
        ir_optimized = stat(parsed[0], parsed[3])
        legacy_optimized = stat(parsed[1], parsed[4])
        legacy = stat(parsed[2], parsed[5])
        fname = f"`{fname.split('/', 3)[-1]}`"
        average = ((
            percent_or_zero(parsed[0], parsed[3]) +
            percent_or_zero(parsed[1], parsed[4]) +
            percent_or_zero(parsed[2], parsed[5])
        ) / 3)
        table += [[average, fname, ir_optimized, legacy_optimized, legacy]]

    sorted_table = [row[1:] for row in sorted(table, reverse=True)]

    if table:
        print("<details><summary>Click for a table of gas differences</summary>\n")
        table_header = ["File name", "IR optimized", "Legacy optimized", "Legacy"]
        print(tabulate(sorted_table, headers=table_header, tablefmt="github"))
        print("</details>")
    else:
        print("No differences found.")

def main():
    parser = ArgumentParser(description=__doc__, formatter_class=RawDescriptionHelpFormatter)
    parser.add_argument(
        '--base',
        dest='base_branch',
        default='origin/develop',
        help='The base branch to diff against. default: origin/develop',
    )
    options = parser.parse_args()

    try:
        semantictest_statistics(options.base_branch)
    except subprocess.CalledProcessError as exception:
        sys.exit(f"Error in the git diff:\n{exception.output}")
    except ParseError as exception:
        sys.exit(
            f"ParseError: {exception}\n\n"
            f"{exception.text}\n"
            f"{' ' * exception.index}^\n"
        )

if __name__ == "__main__":
    main()
