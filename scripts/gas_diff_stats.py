"""A script to collect gas statistics and print it.

Useful to summarize gas differences to semantic tests for a PR / branch.

Dependencies: Parsec (https://pypi.org/project/parsec/) and Tabulate
(https://pypi.org/project/tabulate/)

  pip install parsec tabulate

Run from root project dir.

  python3 scripts/gas_diff_stats.py

Note that the changes to semantic tests have to be committed.

Assumes that there is a remote named ``origin`` pointing to the Solidity github
repository. The changes are compared against ``origin/develop``.

"""
import subprocess
from pathlib import Path
from enum import Enum
from parsec import generate, ParseError, regex, string
from tabulate import tabulate

class Kind(Enum):
    IrOptimized = 1
    Legacy = 2
    LegacyOptimized = 3

class Diff(Enum):
    Minus = 1
    Plus = 2

minus = string("-").result(Diff.Minus)
plus = string("+").result(Diff.Plus)

space = string(" ")
comment = string("//")
colon = string(":")

gas_ir_optimized = string("gas irOptimized").result(Kind.IrOptimized)
gas_legacy_optimized = string("gas legacyOptimized").result(Kind.LegacyOptimized)
gas_legacy = string("gas legacy").result(Kind.Legacy)

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
    codegen_kind = yield gas_ir_optimized ^ gas_legacy_optimized ^ gas_legacy
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
        raise Exception("Empty list")

    def try_parse(line):
        try:
            return diff_string.parse(line)
        except ParseError:
            pass
        return None

    out = [parsed for line in lines if (parsed := try_parse(line)) is not None]
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

def semantictest_statistics():
    """Prints the tabulated statistics that can be pasted in github."""
    def try_parse_git_diff(fname):
        try:
            diff_output = subprocess.check_output(
                "git diff --unified=0 origin/develop HEAD " + fname,
                shell=True,
                universal_newlines=True
            ).splitlines()
            if diff_output:
                return collect_statistics(diff_output)
        except subprocess.CalledProcessError as e:
            print("Error in the git diff:")
            print(e.output)
        return None
    def stat(old, new):
        return ((new - old) / old) * 100  if old else 0

    table = []

    for path in Path("test/libsolidity/semanticTests").rglob("*.sol"):
        fname = path.as_posix()
        parsed = try_parse_git_diff(fname)
        if parsed is None:
            continue
        ir_optimized = stat(parsed[0], parsed[3])
        legacy_optimized = stat(parsed[1], parsed[4])
        legacy = stat(parsed[2], parsed[5])
        fname = fname.split('/', 3)[-1]
        table += [map(str, [fname, ir_optimized, legacy_optimized, legacy])]

    if table:
        print("<details><summary>Click for a table of gas differences</summary>\n")
        table_header = ["File name", "IR-optimized (%)", "Legacy-Optimized (%)", "Legacy (%)"]
        print(tabulate(table, headers=table_header, tablefmt="github"))
        print("</details>")
    else:
        print("No differences found.")

if __name__ == "__main__":
    semantictest_statistics()
