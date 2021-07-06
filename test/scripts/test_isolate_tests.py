#!/usr/bin/env python

import unittest

from unittest_helpers import FIXTURE_DIR, load_fixture

# NOTE: This test file file only works with scripts/ added to PYTHONPATH so pylint can't find the imports
# pragma pylint: disable=import-error
from isolate_tests import extract_docs_cases
# pragma pylint: enable=import-error


CODE_BLOCK_RST_PATH = FIXTURE_DIR / 'code_block.rst'
CODE_BLOCK_RST_CONTENT = load_fixture(CODE_BLOCK_RST_PATH)
CODE_BLOCK_WITH_DIRECTIVES_RST_PATH = FIXTURE_DIR / 'code_block_with_directives.rst'
CODE_BLOCK_WITH_DIRECTIVES_RST_CONTENT = load_fixture(CODE_BLOCK_WITH_DIRECTIVES_RST_PATH)


class TestExtractDocsCases(unittest.TestCase):
    def setUp(self):
        self.maxDiff = 10000

    def test_solidity_block(self):
        expected_cases = [
            "    // SPDX-License-Identifier: GPL-3.0\n"
            "    pragma solidity >=0.7.0 <0.9.0;\n"
            "\n"
            "    contract C {\n"
            "        function foo() public view {}\n"
            "    }\n"
            "\n"
            "\n",

            "    contract C {}\n"
            "\n",
        ]

        self.assertEqual(extract_docs_cases(CODE_BLOCK_RST_PATH), expected_cases)

    def test_solidity_block_with_directives(self):
        expected_cases = [
            "    // SPDX-License-Identifier: GPL-3.0\n"
            "    pragma solidity >=0.7.0 <0.9.0;\n"
            "\n"
            "    contract C {\n"
            "        function foo() public view {}\n"
            "    }\n"
            "\n"
            "\n",

            "    contract C {}\n"
            "\n",

            "    contract D {}\n"
            "    :linenos:\n"
            "\n",

            "    contract E {}\n"
            "\n",
        ]

        self.assertEqual(extract_docs_cases(CODE_BLOCK_WITH_DIRECTIVES_RST_PATH), expected_cases)
