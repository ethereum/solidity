#!/usr/bin/env python3

# Runs all tests for Python-based scripts.
# Any Python test suites located in test/scripts/ will be executed automatically by this script.

import sys
from pathlib import Path
from unittest import TestLoader, TextTestRunner

SCRIPTS_DIR = Path(__file__).parent.parent / "scripts"
TEST_DIR = Path(__file__).parent.parent / "test/scripts/"


if __name__ == '__main__':
    # Add the directory containing scripts to be tested to PYTHONPATH. This means that these
    # scripts can be imported from anywhere (in particular from the test suites) as if they were
    # installed globally. This is necessary because scripts and their tests are in separate
    # directories not recognized by Python as a part of the same package (i.e. their common parent
    # directory does not have an __init__.py file).
    # NOTE: This does not play well with relative imports from test suites so the suites must be
    # placed directly in TEST_DIR and not in its subdirectories. Relative imports from scripts
    # themselves work fine though.
    sys.path.insert(0, str(SCRIPTS_DIR))

    # This is equivalent to `python -m unittest discover --start-directory $TEST_DIR`
    test_suite = TestLoader().discover(start_dir=TEST_DIR)
    result = TextTestRunner().run(test_suite)

    if len(result.errors) > 0 or len(result.failures) > 0:
        sys.exit(1)
