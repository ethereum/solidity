#!/usr/bin/env python3

import re
import os
import sys


def main():
    base_path = os.path.dirname(__file__)
    e2e_path = base_path + "/extracted/libsolidity/semanticTests/end-to-end/"
    tests = []
    for f in os.listdir(e2e_path):
        if f.endswith(".sol"):
            tests.append(f.replace(".sol", ""))

    cpp_file = open(base_path + "/../../test/libsolidity/SolidityEndToEndTest.cpp", "r")
    inside_test = False
    test_name = "'"
    ignore = False
    new_lines = 0
    for line in cpp_file.readlines():
        test = re.search(r'BOOST_AUTO_TEST_CASE\((.*)\)', line, re.M | re.I)
        if test:
            test_name = test.group(1)
            inside_test = True
        ignore = inside_test & (test_name in tests)
        if not ignore:
            if line == "\n":
                new_lines = new_lines + 1
            else:
                new_lines = 0
            if new_lines <= 1:
                sys.stdout.write(line)

        if line == "}\n":
            inside_test = False

    cpp_file.close()
    sys.stdout.flush()


if __name__ == "__main__":
    main()
