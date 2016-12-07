#!/usr/bin/python
#
# This script reads C++ source files and writes all
# multi-line strings into individual files.
# This can be used to extract the Solidity test cases
# into files for e.g. fuzz testing as
# scripts/isolate_tests.py test/libsolidity/*

import sys


def extract_cases(path):
    lines = open(path).read().splitlines()

    inside = False
    tests = []

    for l in lines:
      if inside:
        if l.strip().endswith(')";'):
          inside = False
        else:
          tests[-1] += l + '\n'
      else:
        if l.strip().endswith('R"('):
          inside = True
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
