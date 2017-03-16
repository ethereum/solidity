#!/usr/bin/python
#
# This script reads C++ source files and writes all
# multi-line strings into individual files.
# This can be used to extract the Solidity test cases
# into files for e.g. fuzz testing as
# scripts/isolate_tests.py test/libsolidity/*

import sys
import re


def extract_cases(path):
    lines = open(path).read().splitlines()

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


def write_cases(tests, start=0):
    for i, test in enumerate(tests, start=start):
        open('test%d.sol' % i, 'w').write(test)


if __name__ == '__main__':
    files = sys.argv[1:]

    i = 0
    for path in files:
        cases = extract_cases(path)
        write_cases(cases, start=i)
        i += len(cases)
