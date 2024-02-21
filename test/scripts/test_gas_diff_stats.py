#!/usr/bin/env python

import unittest
from textwrap import dedent

# NOTE: This test file file only works with scripts/ added to PYTHONPATH so pylint can't find the imports
# pragma pylint: disable=import-error
from gas_diff_stats import collect_statistics
# pragma pylint: enable=import-error

class TestGasDiffStats(unittest.TestCase):
    def test_collect_statistics_should_fail_on_empty_diff(self):
        with self.assertRaises(RuntimeError):
            self.assertEqual(collect_statistics(""), (0, 0, 0, 0, 0, 0))

    def test_collect_statistics_should_accept_whitespace_only_diff(self):
        # TODO: Should it really work this way?
        # If we're rejecting empty diff, not sure why whitespace is accepted.
        self.assertEqual(collect_statistics("\n"), (0, 0, 0, 0, 0, 0))
        self.assertEqual(collect_statistics("\n  \n\t\n\n"), (0, 0, 0, 0, 0, 0))

    def test_collect_statistics_should_report_sum_of_gas_costs(self):
        diff_output = dedent("""
            diff --git a/test/libsolidity/semanticTests/various/staticcall_for_view_and_pure.sol b/test/libsolidity/semanticTests/various/staticcall_for_view_and_pure.sol
            index 1306529d4..77a330f3c 100644
            --- a/test/libsolidity/semanticTests/various/staticcall_for_view_and_pure.sol
            +++ b/test/libsolidity/semanticTests/various/staticcall_for_view_and_pure.sol
            @@ -38 +38,2 @@ contract D {
            -// gas legacy: 102095
            +// gas legacy: 76495
            @@ -40,3 +41,6 @@ contract D {
            -// gas irOptimized: 98438588
            -// gas legacy: 98438774
            -// gas legacyOptimized: 98438580
            +// gas irOptimized: 25388
            +// gas legacy: 98413174
            +// gas legacyOptimized: 25380
            @@ -44,3 +48,6 @@ contract D {
            -// gas irOptimized: 98438589
            -// gas legacy: 98438774
            -// gas legacyOptimized: 98438580
            +// gas irOptimized: 25389
            +// gas legacy: 98413174
            +// gas legacyOptimized: 25380
        """).splitlines()

        self.assertEqual(collect_statistics(diff_output), (
            98438588 + 98438589,          # -irOptimized
            98438580 + 98438580,          # -legacyOptimized
            102095 + 98438774 + 98438774, # -legacy
            25388 + 25389,                # +irOptimized
            25380 + 25380,                # +legacyOptimized
            76495 + 98413174 + 98413174,  # +legacy
        ))

    def test_collect_statistics_should_ignore_ir_costs(self):
        diff_output = dedent("""
            -// gas legacy: 1
            -// gas ir: 2
            +// gas legacy: 3
            +// gas ir: 4
        """).splitlines()

        self.assertEqual(collect_statistics(diff_output), (
            0, # -irOptimized
            0, # -legacyOptimized
            1, # -legacy
            0, # +irOptimized
            0, # +legacyOptimized
            3, # +legacy
        ))

    def test_collect_statistics_should_ignore_unchanged_costs(self):
        diff_output = dedent("""
            -// gas legacy: 1
             // gas legacyOptimized: 2
            +// gas legacy: 3
        """).splitlines()

        self.assertEqual(collect_statistics(diff_output), (
            0, # -irOptimized
            0, # -legacyOptimized
            1, # -legacy
            0, # +irOptimized
            0, # +legacyOptimized
            3, # +legacy
        ))

    def test_collect_statistics_should_include_code_deposit_in_total_cost(self):
        diff_output = dedent("""
            -// gas irOptimized: 1
            -// gas legacy: 20
            -// gas legacyOptimized: 300
            +// gas irOptimized: 4000
            +// gas irOptimized code: 50000
            +// gas legacy: 600000
            +// gas legacyOptimized: 7000000
            +// gas legacyOptimized code: 80000000
            -// gas legacy code: 900000000
        """).splitlines()

        self.assertEqual(collect_statistics(diff_output), (
            1,         # -irOptimized
            300,       # -legacyOptimized
            900000020, # -legacy
            54000,     # +irOptimized
            87000000,  # +legacyOptimized
            600000,    # +legacy
        ))
