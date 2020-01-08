#!/usr/bin/env python2

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
          tests[-1] += l.strip()[:-(3 + len(delimiter))]
          inside = False
        else:
          tests[-1] += l + '\n'
      else:
        m = re.search(r'R"([^(]*)\((.*)$', l.strip())
        if m:
          inside = True
          delimiter = m.group(1)
          tests += [m.group(2)]

    return tests

def extract_and_write(f, path):
    if f.endswith('.sol'):
        cases = [open(path, 'r').read()]
    else:
        cases = extract_test_cases(path)
    write_cases(f, cases)

def write_cases(f, tests):
    cleaned_filename = f.replace(".","_").replace("-","_").replace(" ","_").lower()
    for test in tests:
        remainder = re.sub(r'^ {4}', '', test, 0, re.MULTILINE)
        open('test_%s_%s.sol' % (hashlib.sha256(test).hexdigest(), cleaned_filename), 'w').write(remainder)


if __name__ == '__main__':
    path = sys.argv[1]

    for root, subdirs, files in os.walk(path):
        if '_build' in subdirs:
            subdirs.remove('_build')
        for f in files:
            path = join(root, f)
            extract_and_write(f, path)
