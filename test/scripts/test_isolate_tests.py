#!/usr/bin/env python

import unittest

from textwrap import dedent, indent

from unittest_helpers import FIXTURE_DIR, load_fixture

# NOTE: This test file file only works with scripts/ added to PYTHONPATH so pylint can't find the imports
# pragma pylint: disable=import-error
from isolate_tests import extract_solidity_docs_cases, extract_yul_docs_cases
# pragma pylint: enable=import-error

CODE_BLOCK_RST_PATH = FIXTURE_DIR / 'code_block.rst'
CODE_BLOCK_RST_CONTENT = load_fixture(CODE_BLOCK_RST_PATH)
CODE_BLOCK_WITH_DIRECTIVES_RST_PATH = FIXTURE_DIR / 'code_block_with_directives.rst'
CODE_BLOCK_WITH_DIRECTIVES_RST_CONTENT = load_fixture(CODE_BLOCK_WITH_DIRECTIVES_RST_PATH)

def formatCase(text):
    """Formats code to contain only one indentation and terminate with a \n"""
    return indent(dedent(text.lstrip("\n")), "    ") + "\n"

class TestExtractDocsCases(unittest.TestCase):
    def setUp(self):
        self.maxDiff = 10000


    def test_solidity_block(self):
        expected_cases = [formatCase(case) for case in [
            """
                // SPDX-License-Identifier: GPL-3.0
                pragma solidity >=0.7.0 <0.9.0;

                contract C {
                    function foo() public view {}
                }

            """,
            """
                contract C {}
            """,
        ]]

        self.assertEqual(extract_solidity_docs_cases(CODE_BLOCK_RST_PATH), expected_cases)

    def test_solidity_block_with_directives(self):
        expected_cases = [formatCase(case) for case in [
            """
                // SPDX-License-Identifier: GPL-3.0
                pragma solidity >=0.7.0 <0.9.0;

                contract C {
                    function foo() public view {}
                }

            """,
            """
                contract C {}
            """,
            """
                contract D {}
                :linenos:
            """,
            """
                contract E {}
            """,
        ]]

        self.assertEqual(extract_solidity_docs_cases(CODE_BLOCK_WITH_DIRECTIVES_RST_PATH), expected_cases)

    def test_yul_block(self):
        expected_cases = [formatCase(case) for case in [
            """
            {
                let x := add(1, 5)
            }
            """,
            """
            // Yul code wrapped in object
            {
                {
                    let y := mul(3, 5)
                }
            }
            """,
            """
            // Yul code wrapped in named object
            object "Test" {
                {
                    let y := mul(6, 9)
                }
            }
            """,
        ]]

        self.assertEqual(extract_yul_docs_cases(CODE_BLOCK_RST_PATH), expected_cases)

    def test_yul_block_with_directives(self):
        expected_cases = [formatCase(case) for case in [
            """
            {
                let x := add(1, 5)
            }
            """,
            """
            // Yul code wrapped in object
            {
                let y := mul(3, 5)
            }
            """,
            """
            // Yul code wrapped in named object
            object "Test" {
                let y := mul(3, 5)
            :linenos:
            }
            """,
        ]]

        self.assertEqual(extract_yul_docs_cases(CODE_BLOCK_WITH_DIRECTIVES_RST_PATH), expected_cases)
