#!/usr/bin/python
#
# This script reads C++ source files and writes all
# multi-line strings into individual files.
# This can be used to extract the Solidity test cases
# into files for e.g. fuzz testing as
# scripts/isolate_tests.py test/libsolidity/*

import sys
import re
import os
import hashlib
from os.path import join

def extract_cases(path):
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


def write_cases(tests):
    for test in tests:
        open('test_%s.sol' % hashlib.sha256(test).hexdigest(), 'wb').write(test)

if __name__ == '__main__':
    path = sys.argv[1]

    for root, dir, files in os.walk(path):
        for f in files:
            cases = extract_cases(join(root, f))
            write_cases(cases)
