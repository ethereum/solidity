#!/usr/bin/env python2
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
from os.path import join, isfile

def extract_test_cases(path):
    lines = open(path, 'rb').read().splitlines()

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
    inside = False
    tests = []

    # Collect all snippets of indented blocks
    for l in open(path, 'rb').read().splitlines():
        if l != '':
            if not inside and l.startswith(' '):
                # start new test
                tests += ['']
            inside = l.startswith(' ')
        if inside:
            tests[-1] += l + '\n'
    # Filter all tests that do not contain Solidity
    return [
        test for test in tests
        if re.search(r'^    [ ]*(pragma solidity|contract |library |interface )', test, re.MULTILINE)
    ]

def write_cases(f, tests):
    cleaned_filename = f.replace(".","_").replace("-","_").replace(" ","_").lower()
    for test in tests:
        # When code examples are extracted they indented by 8 spaces, which violates the style guide,
        # so before checking remove 4 spaces from each line.
        remainder = re.sub(r'^ {4}', '', test, 0, re.MULTILINE)
        open('test_%s_%s.sol' % (hashlib.sha256(test).hexdigest(), cleaned_filename), 'wb').write(remainder)

def extract_and_write(f, path):
        if docs:
            cases = extract_docs_cases(path)
        else:
            if f.endswith('.sol'):
                cases = [open(path, 'r').read()]
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
                path = join(root, f)
                extract_and_write(f, path)
