#!/usr/bin/env python3
#
# This script reads C++ or RST source files and writes all
# multi-line strings into individual files.
# This can be used to extract the Solidity test cases
# into files for e.g. fuzz testing as
# scripts/isolate_tests.py test/libsolidity/*

import re
import os
import hashlib
from os.path import join, isfile, basename
from argparse import ArgumentParser
from textwrap import indent, dedent

def extract_test_cases(path):
    with open(path, encoding="utf8", errors='ignore', mode='r', newline='') as file:
        lines = file.read().splitlines()

    inside = False
    delimiter = ''
    tests = []

    for l in lines:
        if inside:
            if l.strip().endswith(')' + delimiter + '";'):
                inside = False
            else:
                tests[-1] += l + '\n'
        else:
            m = re.search(r'R"([^(]*)\($', l.strip())
            if m:
                inside = True
                delimiter = m.group(1)
                tests += ['']

    return tests

def extract_solidity_docs_cases(path):
    tests = extract_docs_cases(path, [".. code-block:: solidity", '::'])

    codeStart = "(// SPDX-License-Identifier:|pragma solidity|contract.*{|library.*{|interface.*{)"

    # Filter out tests that are not supposed to be compilable.
    return [
        test.lstrip("\n")
        for test in tests
        if re.search(r'^\s{4}' + codeStart, test, re.MULTILINE) is not None
    ]

def extract_yul_docs_cases(path):
    tests = extract_docs_cases(path, [".. code-block:: yul"])

    def wrap_in_object(code):
        for line in code.splitlines():
            line = line.lstrip()
            if line.startswith("//"):
                continue
            if not line.startswith("object") and not line.startswith("{"):
                return indent(f"{{\n{code.rstrip()}\n}}\n\n", "    ")
            break

        return code

    return [
        wrap_in_object(test)
        for test in tests
        if test.strip() != ""
    ]

# Extract code examples based on the 'beginMarker' parameter
# up until we reach EOF or a line that is not empty and doesn't start with 4
# spaces.
def extract_docs_cases(path, beginMarkers):
    immediatelyAfterMarker = False
    insideBlock = False
    tests = []

    # Collect all snippets of indented blocks
    with open(path, mode='r', errors='ignore', encoding='utf8', newline='') as f:
        lines = f.read().splitlines()

    for line in lines:
        if insideBlock:
            if immediatelyAfterMarker:
                # Skip Sphinx instructions and empty lines between them
                if line == '' or line.lstrip().startswith(":"):
                    continue

            if line == '' or line.startswith(" "):
                tests[-1] += line + "\n"
                immediatelyAfterMarker = False
                continue

            insideBlock = False
        if any(map(line.lower().startswith, beginMarkers)):
            insideBlock = True
            immediatelyAfterMarker = True
            tests += ['']

    return tests

def write_cases(f, solidityTests, yulTests):
    cleaned_filename = f.replace(".","_").replace("-","_").replace(" ","_").lower()
    for language, test in [("sol", t) for t in solidityTests] + [("yul", t) for t in yulTests]:
        # When code examples are extracted they are indented by 8 spaces, which violates the style guide,
        # so before checking remove 4 spaces from each line.
        remainder = dedent(test)
        source_code_hash = hashlib.sha256(test.encode("utf-8")).hexdigest()
        sol_filename = f'test_{source_code_hash}_{cleaned_filename}.{language}'
        with open(sol_filename, mode='w', encoding='utf8', newline='') as fi:
            fi.write(remainder)

def extract_and_write(path, language):
    assert language in ["solidity", "yul", ""]
    yulCases = []
    cases = []

    if path.lower().endswith('.rst'):
        if language in ("solidity", ""):
            cases = extract_solidity_docs_cases(path)

        if language in ("yul", ""):
            yulCases  = extract_yul_docs_cases(path)
    elif path.endswith('.sol'):
        if language in ("solidity", ""):
            with open(path, mode='r', encoding='utf8', newline='') as f:
                cases = [f.read()]
    else:
        cases = extract_test_cases(path)

    write_cases(basename(path), cases, yulCases)

if __name__ == '__main__':
    script_description = (
        "Reads Solidity, C++ or RST source files and extracts compilable solidity and yul code blocks from them. "
        "Can be used to generate test cases to validate code examples. "
    )

    parser = ArgumentParser(description=script_description)
    parser.add_argument(dest='path', help='Path to file or directory to look for code in.')
    parser.add_argument(
        '-l', '--language',
        dest='language',
        choices=["yul", "solidity"],
        default="",
        action='store',
        help="Extract only code blocks in the given language"
    )
    options = parser.parse_args()
    path = options.path

    if isfile(path):
        extract_and_write(path, options.language)
    else:
        for root, subdirs, files in os.walk(path):
            if '_build' in subdirs:
                subdirs.remove('_build')
            if 'compilationTests' in subdirs:
                subdirs.remove('compilationTests')
            for f in files:
                if basename(f) == "invalid_utf8_sequence.sol":
                    continue  # ignore the test with broken utf-8 encoding
                path = join(root, f)
                extract_and_write(path, options.language)
