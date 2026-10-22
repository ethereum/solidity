#!/usr/bin/env python3
"""
Compare Yul test expected outputs between base and PR branch using yuldiff.
Handles:
  - yulOptimizerTests/*.yul (expected output after "// step:" header)
  - cmdlineTests/*/output (plain text with Yul objects)
  - cmdlineTests/*/output.json (Yul embedded in JSON string values)
"""

import argparse
import enum
import json
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path


class FileType(enum.Enum):
    YUL_OPTIMIZER_TEST = enum.auto()
    CMDLINE_OUTPUT_JSON = enum.auto()
    CMDLINE_OUTPUT_TEXT = enum.auto()


class CompareStatus(enum.Enum):
    EQUIVALENT = enum.auto()
    MISMATCH = enum.auto()
    YULDIFF_ERROR = enum.auto()
    TIMEOUT = enum.auto()
    ERROR = enum.auto()


@dataclass
class CompareResult:
    status: CompareStatus
    message: str = ""


def git_show(ref, path):
    return subprocess.check_output(
        ["git", "show", f"{ref}:{path}"], text=True
    )


def sanitize_yul(source):
    """Replace unparsable test placeholders."""
    return source.replace('hex"<BYTECODE REMOVED>"', 'hex""')


def run_yuldiff(yuldiff_binary, yul_a: str, yul_b: str) -> CompareResult:
    """Run yuldiff on two Yul source strings."""
    yul_a = sanitize_yul(yul_a)
    yul_b = sanitize_yul(yul_b)
    with (
        tempfile.NamedTemporaryFile(mode="w", suffix=".yul") as fa,
        tempfile.NamedTemporaryFile(mode="w", suffix=".yul") as fb,
    ):
        fa.write(yul_a)
        fb.write(yul_b)
        fa.flush()
        fb.flush()
        try:
            result = subprocess.run(
                [yuldiff_binary, fa.name, fb.name],
                capture_output=True, text=True, timeout=10, check=False
            )
            if result.returncode == 0:
                return CompareResult(CompareStatus.EQUIVALENT)
            msg = result.stdout.strip()
            if len(msg) == 0 and len(result.stderr.strip()) > 0:
                return CompareResult(CompareStatus.YULDIFF_ERROR, result.stderr.strip())
            return CompareResult(CompareStatus.MISMATCH, msg if len(msg) > 0 else "unknown error")
        except subprocess.TimeoutExpired:
            return CompareResult(CompareStatus.TIMEOUT)
        except OSError as e:
            return CompareResult(CompareStatus.ERROR, str(e))


def extract_optimizer_expected(content):
    """Extract expected output from yulOptimizerTests .yul files."""
    separator = "// ----"
    if separator not in content:
        return []
    after_separator = content.split(separator, 1)[1]

    comment_lines = []
    for line in after_separator.split("\n")[1:]:
        if line.startswith("// "):
            comment_lines.append(line[3:])
        elif line.strip() == "//":
            comment_lines.append("")
        else:
            break
    # First line is "step: <name>", followed by an empty line, then the Yul code.
    if len(comment_lines) < 3 or not comment_lines[0].startswith("step:"):
        return []
    yul = "\n".join(comment_lines[2:]).strip()
    if len(yul) == 0:
        return []
    return [f'object "test" {{ code {{\n{yul}\n}} }}']


# All section headers the CLI can emit (from CommandLineInterface.cpp).
_KNOWN_SECTION_HEADERS = {
    "IR:", "IR AST:", "Optimized IR:", "Optimized IR AST:",
    "Yul Control Flow Graph:",
    "EVM assembly:", "Binary:", "Binary of the runtime part:",
    "Opcodes:", "Binary representation:", "Text representation:",
    "AST:", "JSON AST (compact format):",
    "Metadata:", "Contract JSON ABI",
    "Contract Storage Layout:", "Contract Transient Storage Layout:",
    "Gas estimation:", "Function signatures:",
    "Error signatures:", "Event signatures:",
    "Pretty printed source:",
    "Debug Data (ethdebug/format/program):",
    "Debug Data of the runtime part (ethdebug/format/program):",
}

_YUL_SECTION_HEADERS = {"IR:", "Optimized IR:", "Pretty printed source:"}


def _is_section_header(line: str) -> bool:
    """Check if a line is a cmdline output section header."""
    stripped = line.strip()
    if stripped.startswith("=======") and stripped.endswith("======="):
        return True
    return stripped in _KNOWN_SECTION_HEADERS


def extract_yul_objects_from_cmdline_output_text(content: str) -> list[str]:
    """Extract Yul objects from cmdline output text files.

    Parses section headers and collects content under IR/Optimized IR/Pretty printed source.
    """
    objects = []
    current_lines: list[str] | None = None

    for line in content.split("\n"):
        if _is_section_header(line):
            if current_lines is not None and len(current_lines) > 0:
                objects.append("\n".join(current_lines))

            if line.strip() in _YUL_SECTION_HEADERS:
                current_lines = []
            else:
                current_lines = None
        elif current_lines is not None:
            current_lines.append(line)

    if current_lines is not None and len(current_lines) > 0:
        objects.append("\n".join(current_lines))

    return objects


_YUL_JSON_KEYS = ("ir", "irOptimized")


def extract_yul_from_output_json(content):
    """Extract Yul object strings from Standard JSON output files.

    Yul IR lives at contracts.<file>.<contract>.ir and .irOptimized.
    """
    data = json.loads(content, strict=False)
    yul_strings = []
    contracts = data.get("contracts", {})
    for source_units in contracts.values():
        for contract in source_units.values():
            for key in _YUL_JSON_KEYS:
                yul = contract.get(key, "")
                if isinstance(yul, str) and len(yul.strip()) > 0:
                    yul_strings.append(yul.strip())
    return yul_strings


def get_changed_files(base_ref, pr_ref):
    """Return (modified, added, deleted) file lists between two refs."""
    def diff_filter(filt):
        output = subprocess.check_output(
            ["git", "diff", "--name-only", f"--diff-filter={filt}", base_ref, pr_ref], text=True
        ).strip()
        if len(output) == 0:
            return []
        return output.split("\n")
    return diff_filter("M"), diff_filter("A"), diff_filter("D")


_YUL_OPTIMIZER_TEST_DIRS = {
    "yulOptimizerTests",
    "yulControlFlowGraph",
    "yulStackLayout"
}


def classify_file(path_str: str) -> FileType | None:
    path = Path(path_str)
    if path.suffix == ".yul" and any(d in path.parts for d in _YUL_OPTIMIZER_TEST_DIRS):
        return FileType.YUL_OPTIMIZER_TEST
    if "cmdlineTests" in path.parts:
        if path.name == "output.json":
            return FileType.CMDLINE_OUTPUT_JSON
        if path.name in ("output", "err"):
            return FileType.CMDLINE_OUTPUT_TEXT
    return None


def extract_yul(content: str, file_type: FileType) -> list[str]:
    match file_type:
        case FileType.YUL_OPTIMIZER_TEST:
            return extract_optimizer_expected(content)
        case FileType.CMDLINE_OUTPUT_JSON:
            return extract_yul_from_output_json(content)
        case FileType.CMDLINE_OUTPUT_TEXT:
            return extract_yul_objects_from_cmdline_output_text(content)
        case _:
            raise ValueError(f"Unhandled file type: {file_type}")


def fetch_pr_ref(pr_id, remote):
    """Fetch a PR and return a local ref for it."""
    ref = f"{remote}/pr-{pr_id}"
    subprocess.check_call(["git", "fetch", remote, f"pull/{pr_id}/head:{ref}"])
    return ref


def find_base_ref(pr_ref, base_branch):
    return subprocess.check_output(
        ["git", "merge-base", base_branch, pr_ref], text=True
    ).strip()


def main():
    parser = argparse.ArgumentParser(
        description="Compare Yul test expected outputs between base and PR using yuldiff."
    )
    parser.add_argument("yuldiff", help="Path to the yuldiff binary")
    parser.add_argument("pr", type=int, nargs="?", default=None, help="PR number to compare (omit to compare HEAD against base)")
    parser.add_argument(
        "--base", "-b",
        default="origin/develop",
        help="Base branch for merge-base calculation (default: origin/develop)",
    )
    parser.add_argument(
        "--remote", "-r",
        default="origin",
        help="Git remote to fetch PRs from (default: origin)",
    )
    args = parser.parse_args()

    yuldiff_binary = Path(args.yuldiff)
    if not yuldiff_binary.exists():
        parser.error(f"yuldiff binary not found: {yuldiff_binary}")

    if args.pr is not None:
        pr_ref = fetch_pr_ref(args.pr, args.remote)
        label = f"PR #{args.pr}: {pr_ref}"
    else:
        pr_ref = "HEAD"
        label = f"HEAD ({subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], text=True).strip()})"
    base_ref = find_base_ref(pr_ref, args.base)

    print(f"{label} (base: {base_ref})")

    modified, added, deleted = get_changed_files(base_ref, pr_ref)
    all_files = modified + added + deleted

    equivalent = 0
    mismatches = []
    errors = []
    skipped = []

    test_files = []
    classified = {}
    for f in all_files:
        ftype = classify_file(f)
        if ftype:
            test_files.append((f, ftype))
            classified[f] = ftype

    added_set = set(added)
    deleted_set = set(deleted)

    print("Changed files:")
    for f in all_files:
        if f in classified:
            print(f"  \033[92m{f}\033[0m [{classified[f].name}]")
        else:
            print(f"  {f}")

    print(f"\n{len(test_files)} of {len(all_files)} files have comparable Yul content\n")

    for filepath, ftype in sorted(test_files):
        if filepath in added_set or filepath in deleted_set:
            reason = "added" if filepath in added_set else "deleted"
            skipped.append((filepath, reason))
            continue

        base_content = git_show(base_ref, filepath)
        pr_content = git_show(pr_ref, filepath)

        if base_content == pr_content:
            equivalent += 1
            continue

        base_yuls = extract_yul(base_content, ftype)
        pr_yuls = extract_yul(pr_content, ftype)

        if len(base_yuls) == 0 and len(pr_yuls) == 0:
            skipped.append((filepath, "no Yul objects extracted"))
            continue

        if len(base_yuls) != len(pr_yuls):
            mismatches.append((filepath, f"different number of Yul objects: {len(base_yuls)} vs {len(pr_yuls)}"))
            continue

        file_ok = True
        for idx, (yul_a, yul_b) in enumerate(zip(base_yuls, pr_yuls)):
            if yul_a == yul_b:
                continue

            cmp = run_yuldiff(yuldiff_binary, yul_a, yul_b)
            match cmp.status:
                case CompareStatus.EQUIVALENT:
                    continue
                case CompareStatus.MISMATCH:
                    mismatches.append((filepath, cmp.message))
                case CompareStatus.YULDIFF_ERROR | CompareStatus.TIMEOUT | CompareStatus.ERROR:
                    errors.append((filepath, idx, cmp.message))
                case _:
                    raise ValueError(f"Unhandled compare status: {cmp.status}")
            file_ok = False
            break

        if file_ok:
            equivalent += 1

    print("=" * 50)
    print(f"RESULTS: {len(test_files)} test files")
    print(f"  Equivalent:   {equivalent}")
    print(f"  Mismatched:   {len(mismatches)}")
    print(f"  Errors:       {len(errors)}")
    print(f"  Skipped:      {len(skipped)}")
    print("=" * 50)

    if len(mismatches) > 0:
        print("\nMismatched files:")
        for f, msg in mismatches:
            print(f"  - {f}")
            for line in msg.split("\n"):
                print(f"    {line}")

    if len(errors) > 0:
        print("\nErrors:")
        for f, idx, msg in errors:
            print(f"  - {f} (object {idx}): {msg}")

    if len(skipped) > 0:
        print("\nSkipped files:")
        for f, reason in skipped:
            print(f"  - {f} ({reason})")

    if len(mismatches) > 0 or len(errors) > 0:
        sys.exit(1)


if __name__ == "__main__":
    main()
