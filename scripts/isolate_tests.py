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
# Look for `pragma solidity` and abort a line not indented properly.
# If the comment `// This will not compile` is above the pragma,
# the code is skipped.
def extract_docs_cases(path):
    # Note: this code works, because splitlines() removes empty new lines
    #       and thus even if the empty new lines are missing indentation
    lines = open(path, 'rb').read().splitlines()

    ignore = False
    inside = False
    tests = []

    for l in lines:
      if inside:
        # Abort if indentation is missing
        m = re.search(r'^[^ ]+', l)
        if m:
          inside = False
        else:
          tests[-1] += l + '\n'
      else:
        m = re.search(r'^    // This will not compile', l)
        if m:
          ignore = True

        if ignore:
          # Abort if indentation is missing
          m = re.search(r'^[^ ]+', l)
          if m:
            ignore = False
        else:
          m = re.search(r'^    pragma solidity .*[0-9]+\.[0-9]+\.[0-9]+;$', l)
          if m:
            inside = True
            tests += [l]

    return tests

def write_cases(tests):
    for test in tests:
        open('test_%s.sol' % hashlib.sha256(test).hexdigest(), 'wb').write(test)


def extract_and_write(f, path):
        if docs:
            cases = extract_docs_cases(path)
        else:
            if f.endswith('.sol'):
                cases = [open(path, 'r').read()]
            else:
                cases = extract_test_cases(path)
        write_cases(cases)

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
