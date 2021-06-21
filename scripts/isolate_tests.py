#!/usr/bin/env python3
#
# This script reads C++ or RST source files and writes all
# multi-line strings into individual files.
# This can be used to extract the Solidity test cases
# into files for e.g. fuzz testing as
# scripts/isolate_tests.py test/libsolidity/*

import sys
import re
import os
import hashlib
from os.path import join, isfile, split

def extract_test_cases(path):
    lines = open(path, encoding="utf8", errors='ignore', mode='r', newline='').read().splitlines()

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

# Contract sources are indented by 4 spaces.
# Look for `pragma solidity`, `contract`, `library` or `interface`
# and abort a line not indented properly.
def extract_docs_cases(path):
    insideBlock = False
    insideBlockParameters = False
    pastBlockParameters = False
    extractedLines = []
    tests = []

    # Collect all snippets of indented blocks
    for l in open(path, mode='r', errors='ignore', encoding='utf8', newline='').read().splitlines():
        if l != '':
            if not insideBlock and l.startswith(' '):
                # start new test
                extractedLines += ['']
                insideBlockParameters = False
                pastBlockParameters = False
            insideBlock = l.startswith(' ')
        if insideBlock:
            if not pastBlockParameters:
                # NOTE: For simplicity this allows blank lines between block parameters even
                # though Sphinx does not. This does not matter since the first non-empty line in
                # a Solidity file cannot start with a colon anyway.
                if not l.strip().startswith(':') and (l != '' or not insideBlockParameters):
                    insideBlockParameters = False
                    pastBlockParameters = True
                else:
                    insideBlockParameters = True

            if not insideBlockParameters:
                extractedLines[-1] += l + '\n'

    codeStart = "(// SPDX-License-Identifier:|pragma solidity|contract.*{|library.*{|interface.*{)"

    # Filter all tests that do not contain Solidity or are indented incorrectly.
    for lines in extractedLines:
        if re.search(r'^\s{0,3}' + codeStart, lines, re.MULTILINE):
            print("Indentation error in " + path + ":")
            print(lines)
            exit(1)
        if re.search(r'^\s{4}' + codeStart, lines, re.MULTILINE):
            tests.append(lines)

    return tests

def write_cases(f, tests):
    cleaned_filename = f.replace(".","_").replace("-","_").replace(" ","_").lower()
    for test in tests:
        # When code examples are extracted they are indented by 8 spaces, which violates the style guide,
        # so before checking remove 4 spaces from each line.
        remainder = re.sub(r'^ {4}', '', test, 0, re.MULTILINE)
        sol_filename = 'test_%s_%s.sol' % (hashlib.sha256(test.encode("utf-8")).hexdigest(), cleaned_filename)
        open(sol_filename, mode='w', encoding='utf8', newline='').write(remainder)

def extract_and_write(f, path):
    if docs:
        cases = extract_docs_cases(path)
    else:
        if f.endswith('.sol'):
            cases = [open(path, mode='r', encoding='utf8', newline='').read()]
        else:
            cases = extract_test_cases(path)
    write_cases(f, cases)

if __name__ == '__main__':
    path = sys.argv[1]
    docs = False
    if len(sys.argv) > 2 and sys.argv[2] == 'docs':
        docs = True

    if isfile(path):
        extract_and_write(path, path)
    else:
        for root, subdirs, files in os.walk(path):
            if '_build' in subdirs:
                subdirs.remove('_build')
            if 'compilationTests' in subdirs:
                subdirs.remove('compilationTests')
            for f in files:
                _, tail = split(f)
                if tail == "invalid_utf8_sequence.sol":
                    continue  # ignore the test with broken utf-8 encoding
                path = join(root, f)
                extract_and_write(f, path)
