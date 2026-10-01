#!/usr/bin/env python3

# Runs the tests validating the compiler's ethdebug output against the ethdebug/format schemas.
# Any test suite located in test/ethdebugSchemaTests/ will be executed automatically by this script.

import sys
from argparse import ArgumentParser
from dataclasses import dataclass
from pathlib import Path
from unittest import TestLoader, TestSuite, TextTestRunner

TEST_DIR = Path(__file__).parent / "ethdebugSchemaTests"


@dataclass(frozen=True)
class Config:
    solc_path: Path


def test_cases(test_suite):
    for test in test_suite:
        if isinstance(test, TestSuite):
            yield from test_cases(test)
        else:
            yield test


if __name__ == '__main__':
    parser = ArgumentParser(description="Validate the compiler's ethdebug output against the ethdebug/format schemas.")
    parser.add_argument("--solc-binary-path", type=Path, required=True, help="Path to the solidity compiler binary.")
    options = parser.parse_args()
    assert options.solc_binary_path.is_file(), f"Not a file: {options.solc_binary_path}"

    config = Config(solc_path=options.solc_binary_path)

    # This is equivalent to `python -m unittest discover --start-directory $TEST_DIR`, with
    # every test case given the configuration.
    test_suite = TestLoader().discover(start_dir=TEST_DIR)
    for test_case in test_cases(test_suite):
        test_case.config = config
    result = TextTestRunner(verbosity=2).run(test_suite)

    if len(result.errors) > 0 or len(result.failures) > 0:
        sys.exit(1)
