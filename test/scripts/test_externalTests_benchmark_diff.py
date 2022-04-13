#!/usr/bin/env python3

from textwrap import dedent
import json
import unittest

from unittest_helpers import FIXTURE_DIR, load_fixture

# NOTE: This test file file only works with scripts/ added to PYTHONPATH so pylint can't find the imports
# pragma pylint: disable=import-error
from externalTests.benchmark_diff import BenchmarkDiffer, DifferenceStyle, DiffTableSet, DiffTableFormatter, OutputFormat
# pragma pylint: enable=import-error

SUMMARIZED_BENCHMARKS_DEVELOP_JSON_PATH = FIXTURE_DIR / 'summarized-benchmarks-develop.json'
SUMMARIZED_BENCHMARKS_BRANCH_JSON_PATH = FIXTURE_DIR / 'summarized-benchmarks-branch.json'

SUMMARIZED_DIFF_HUMANIZED_MD_PATH = FIXTURE_DIR / 'summarized-benchmark-diff-develop-branch-humanized.md'
SUMMARIZED_DIFF_HUMANIZED_MD = load_fixture(SUMMARIZED_DIFF_HUMANIZED_MD_PATH)


class TestBenchmarkDiff(unittest.TestCase):
    def setUp(self):
        self.maxDiff = 10000

    def test_benchmark_diff(self):
        report_before = json.loads(load_fixture(SUMMARIZED_BENCHMARKS_DEVELOP_JSON_PATH))
        report_after = json.loads(load_fixture(SUMMARIZED_BENCHMARKS_BRANCH_JSON_PATH))
        expected_diff = {
            "bleeps": {
                "ir-optimize-evm+yul": {
                    # Numerical difference -> negative/positive/zero.
                    # Zeros are not skipped to differentiate them from missing values.
                    "bytecode_size": 132868 - 132165,
                    "deployment_gas": 0,
                    "method_gas": 39289198 - 39289935,
                },
                "legacy-optimize-evm+yul": {
                    # No differences within preset -> zeros still present.
                    "bytecode_size": 0,
                    "deployment_gas": 0,
                    "method_gas": 0,
                },
            },
            "colony": {
                # Preset missing on one side -> replace dict with string
                "ir-optimize-evm+yul": "!A",
                "legacy-no-optimize": "!B",
                "legacy-optimize-evm+yul": {
                    "bytecode_size": 0,
                    # Attribute missing on both sides -> skip
                    #"deployment_gas":
                    #"method_gas":
                },
            },
            "elementfi": {
                "legacy-no-optimize": {
                    # Attributes null on one side -> replace value with string
                    "bytecode_size": "!A",
                    "deployment_gas": "!B",
                    # Attribute null on both sides -> skip
                    #"method_gas":
                },
                "legacy-optimize-evm+yul": {
                    # Attributes missing on one side -> replace value with string
                    "bytecode_size": "!A",
                    "deployment_gas": "!B",
                    # Attribute missing on both sides -> skip
                    #"method_gas":
                },
                "ir-no-optimize": {
                    # Attributes missing on one side, null on the other -> skip
                    #"bytecode_size":
                    #"deployment_gas":
                    "method_gas": 0,
                },
                # Empty preset missing on one side -> replace dict with string
                "legacy-optimize-evm-only": "!A",
                "ir-optimize-evm-only": "!B",
            },
            "euler": {
                # Matching versions -> show attributes, skip version
                "ir-no-optimize": {
                    "bytecode_size": 328540 - 323909,
                    "deployment_gas": 0,
                    "method_gas": 3537419168 - 3452105184,
                },
                # Different versions, different values -> replace whole preset with string
                "legacy-no-optimize": "!V",
                # Different versions, same values -> replace whole preset with string
                "legacy-optimize-evm+yul": "!V",
                # Different versions (not a commit hash), different values -> replace whole preset with string
                "legacy-optimize-evm-only": "!V",
                # Version missing on one side -> replace whole preset with string
                "ir-optimize-evm-only": "!V",
                # Version missing on both sides -> assume same version
                "ir-optimize-evm+yul": {
                    "bytecode_size": 205211 - 182190,
                    "deployment_gas": 39459629 - 35236828,
                    "method_gas": 0,
                },
            },
            "zeppelin": {
                "legacy-optimize-evm+yul": {
                    # Whole project identical -> attributes still present, with zeros
                    "bytecode_size": 0,
                    "deployment_gas": 0,
                    # Field missing on both sides -> skip
                    #"method_gas":
                }
            },
            # Empty project missing on one side -> replace its dict with a string
            "gnosis": "!B",
            "ens": "!A",
        }
        differ = BenchmarkDiffer(DifferenceStyle.ABSOLUTE, None, OutputFormat.JSON)
        self.assertEqual(differ.run(report_before, report_after), expected_diff)


class TestBenchmarkDiffer(unittest.TestCase):
    def setUp(self):
        self.maxDiff = 10000

    @staticmethod
    def _nest(value, levels):
        nested_value = value
        for level in levels:
            nested_value = {level: nested_value}

        return nested_value

    def _assert_single_value_diff_matches(self, differ, cases, nest_result=True, nestings=None):
        if nestings is None:
            nestings = [[], ['p'], ['p', 's'], ['p', 's', 'a']]

        for levels in nestings:
            for (before, after, expected_diff) in cases:
                self.assertEqual(
                    differ.run(self._nest(before, levels), self._nest(after, levels)),
                    self._nest(expected_diff, levels) if nest_result else expected_diff,
                    f'Wrong diff for {self._nest(before, levels)} vs {self._nest(after, levels)}'
                )

    def test_empty(self):
        for style in DifferenceStyle:
            differ = BenchmarkDiffer(style, None, OutputFormat.JSON)
            self._assert_single_value_diff_matches(differ, [({}, {}, {})], nest_result=False)

    def test_null(self):
        for style in DifferenceStyle:
            differ = BenchmarkDiffer(style, None, OutputFormat.JSON)
            self._assert_single_value_diff_matches(differ, [(None, None, {})], nest_result=False)

    def test_number_diff_absolute_json(self):
        for output_format in OutputFormat:
            self._assert_single_value_diff_matches(
                BenchmarkDiffer(DifferenceStyle.ABSOLUTE, 4, output_format),
                [
                    (2,   2,    0),
                    (2,   5,    3),
                    (5,   2,   -3),
                    (2.0, 2.0,  0),
                    (2,   2.0,  0),
                    (2.0, 2,    0),
                    (2,   2.5,  2.5 - 2),
                    (2.5, 2,    2 - 2.5),

                    (0,   0,    0),
                    (0,   2,    2),
                    (0,   -2,  -2),

                    (-3, -1,    2),
                    (-1, -3,   -2),
                    (2,   0,   -2),
                    (-2,  0,    2),

                    (1.00006, 1,  1 - 1.00006),
                    (1, 1.00006,  1.00006 - 1),
                    (1.00004, 1, 1 - 1.00004),
                    (1, 1.00004, 1.00004 - 1),
                ],
            )

    def test_number_diff_json(self):
        for output_format in OutputFormat:
            self._assert_single_value_diff_matches(
                BenchmarkDiffer(DifferenceStyle.RELATIVE, 4, output_format),
                [
                    (2,   2,   0),
                    (2,   5,   (5 - 2) / 2),
                    (5,   2,   (2 - 5) / 5),
                    (2.0, 2.0, 0),
                    (2,   2.0, 0),
                    (2.0, 2,   0),
                    (2,   2.5, (2.5 - 2) / 2),
                    (2.5, 2,   (2 - 2.5) / 2.5),

                    (0,   0,   0),
                    (0,   2,   '+INF'),
                    (0,   -2,  '-INF'),

                    (-3, -1,   0.6667),
                    (-1, -3,  -2),
                    (2,   0,  -1),
                    (-2,  0,   1),

                    (1.00006, 1,   -0.0001),
                    (1, 1.00006,    0.0001),
                    (1.000004, 1, '-0'),
                    (1, 1.000004, '+0'),
                ],
            )

    def test_number_diff_humanized_json_and_console(self):
        for output_format in [OutputFormat.JSON, OutputFormat.CONSOLE]:
            self._assert_single_value_diff_matches(
                BenchmarkDiffer(DifferenceStyle.HUMANIZED, 4, output_format),
                [
                    (2,   2,      '0%'),
                    (2,   5,   '+150%'),
                    (5,   2,    '-60%'),
                    (2.0, 2.0,    '0%'),
                    (2,   2.0,    '0%'),
                    (2.0, 2,      '0%'),
                    (2,   2.5,  '+25%'),
                    (2.5, 2,    '-20%'),

                    (0,   0,      '0%'),
                    (0,   2,   '+INF%'),
                    (0,   -2,  '-INF%'),

                    (-3, -1, '+66.67%'),
                    (-1, -3,   '-200%'),
                    (2,   0,   '-100%'),
                    (-2,  0,   '+100%'),

                    (1.00006, 1,  '-0.01%'),
                    (1, 1.00006,  '+0.01%'),
                    (1.000004, 1,    '-0%'),
                    (1, 1.000004,    '+0%'),
                ],
            )

    def test_number_diff_humanized_markdown(self):
        self._assert_single_value_diff_matches(
            BenchmarkDiffer(DifferenceStyle.HUMANIZED, 4, OutputFormat.MARKDOWN),
            [
                (2,   2,             '`0%`'),
                (2,   5,   '**`+150% ❌`**'),
                (5,   2,    '**`-60% ✅`**'),
                (2.0, 2.0,           '`0%`'),
                (2,   2.0,           '`0%`'),
                (2.0, 2,             '`0%`'),
                (2,   2.5,  '**`+25% ❌`**'),
                (2.5, 2,    '**`-20% ✅`**'),

                (0,   0,             '`0%`'),
                (0,   2,          '`+INF%`'),
                (0,   -2,         '`-INF%`'),

                (-3, -1, '**`+66.67% ❌`**'),
                (-1, -3,   '**`-200% ✅`**'),
                (2,   0,   '**`-100% ✅`**'),
                (-2,  0,   '**`+100% ❌`**'),

                (1.00006, 1,  '**`-0.01% ✅`**'),
                (1, 1.00006,  '**`+0.01% ❌`**'),
                (1.000004, 1,           '`-0%`'),
                (1, 1.000004,           '`+0%`'),
            ],
        )

    def test_type_mismatch(self):
        for style in DifferenceStyle:
            self._assert_single_value_diff_matches(
                BenchmarkDiffer(style, 4, OutputFormat.JSON),
                [
                    (1, {}, '!T'),
                    ({}, 1, '!T'),
                    (1.5, {}, '!T'),
                    ({}, 1.5, '!T'),
                    ('1', {}, '!T'),
                    ({}, '1', '!T'),
                    (1, '1', '!T'),
                    ('1', 1, '!T'),
                    (1.5, '1', '!T'),
                    ('1', 1.5, '!T'),
                    ('1', '1', '!T'),
                ],
            )

    def test_version_mismatch(self):
        for style in DifferenceStyle:
            self._assert_single_value_diff_matches(
                BenchmarkDiffer(style, 4, OutputFormat.JSON),
                [
                    ({'a': 123, 'version': 1}, {'a': 123, 'version': 2}, '!V'),
                    ({'a': 123, 'version': 2}, {'a': 123, 'version': 1}, '!V'),
                    ({'a': 123, 'version': 'a'}, {'a': 123, 'version': 'b'}, '!V'),
                    ({'a': 123, 'version': 'a'}, {'a': 123, 'version': 1}, '!V'),

                    ({'a': 'a', 'version': 1}, {'a': 'a', 'version': 2}, '!V'),
                    ({'a': {}, 'version': 1}, {'a': {}, 'version': 2}, '!V'),
                    ({'s': {'a': 1}, 'version': 1}, {'s': {'a': 1}, 'version': 2}, '!V'),

                    ({'a': 123, 'version': 1}, {'a': 456, 'version': 2}, '!V'),
                    ({'a': 'a', 'version': 1}, {'a': 'b', 'version': 2}, '!V'),
                    ({'s': {'a': 1}, 'version': 1}, {'s': {'a': 2}, 'version': 2}, '!V'),
                ],
            )

    def test_missing(self):
        for style in DifferenceStyle:
            self._assert_single_value_diff_matches(
                BenchmarkDiffer(style, None, OutputFormat.JSON),
                [
                    (1, None, '!A'),
                    (None, 1, '!B'),
                    ('1', None, '!A'),
                    (None, '1', '!B'),
                    ({}, None, '!A'),
                    (None, {}, '!B'),

                    ({'x': 1}, {}, {'x': '!A'}),
                    ({}, {'x': 1}, {'x': '!B'}),
                    ({'x': 1}, {'x': None}, {'x': '!A'}),
                    ({'x': None}, {'x': 1}, {'x': '!B'}),
                    ({'x': 1}, {'y': 1}, {'x': '!A', 'y': '!B'}),

                    ({'x': {}}, {}, {'x': '!A'}),
                    ({}, {'x': {}}, {'x': '!B'}),
                    ({'p': {'x': {}}}, {}, {'p': '!A'}),
                    ({}, {'p': {'x': {}}}, {'p': '!B'}),
                ],
            )

    def test_missing_vs_null(self):
        for style in DifferenceStyle:
            self._assert_single_value_diff_matches(
                BenchmarkDiffer(style, None, OutputFormat.JSON),
                [
                    ({'a': None}, {}, {}),
                    ({}, {'a': None}, {}),
                ],
                nest_result=False,
            )


class TestDiffTableFormatter(unittest.TestCase):
    def setUp(self):
        self.maxDiff = 10000

        self.report_before = {
            'project A': {
                'preset X': {'A1':  99, 'A2': 50, 'version': 1},
                'preset Y': {'A1':   0, 'A2': 50, 'version': 1},
            },
            'project B': {
                'preset X': {           'A2': 50},
                'preset Y': {'A1':   0},
            },
            'project C': {
                'preset X': {'A1':   0, 'A2': 50, 'version': 1},
            },
            'project D': {
                'preset X': {'A1': 999},
            },
        }
        self.report_after = {
            'project A': {
                'preset X': {'A1': 100, 'A2':  50, 'version': 1},
                'preset Y': {'A1': 500, 'A2': 500, 'version': 2},
            },
            'project B': {
                'preset X': {'A1':   0},
                'preset Y': {           'A2': 50},
            },
            'project C': {
                'preset Y': {'A1':   0, 'A2': 50, 'version': 1},
            },
            'project E': {
                'preset Y': {           'A2': 999},
            },
        }

    def test_diff_table_formatter(self):
        report_before = json.loads(load_fixture(SUMMARIZED_BENCHMARKS_DEVELOP_JSON_PATH))
        report_after = json.loads(load_fixture(SUMMARIZED_BENCHMARKS_BRANCH_JSON_PATH))
        differ = BenchmarkDiffer(DifferenceStyle.HUMANIZED, 4, OutputFormat.MARKDOWN)
        diff = differ.run(report_before, report_after)

        self.assertEqual(DiffTableFormatter.run(DiffTableSet(diff), OutputFormat.MARKDOWN), SUMMARIZED_DIFF_HUMANIZED_MD)

    def test_diff_table_formatter_json_absolute(self):
        differ = BenchmarkDiffer(DifferenceStyle.ABSOLUTE, 4, OutputFormat.JSON)
        diff = differ.run(self.report_before, self.report_after)

        expected_formatted_table = dedent("""\
            {
                "preset X": {
                    "project A": {
                        "A1": 1,
                        "A2": 0
                    },
                    "project B": {
                        "A1": "!B",
                        "A2": "!A"
                    },
                    "project C": {
                        "A1": "!A",
                        "A2": "!A"
                    },
                    "project D": {
                        "A1": "!A",
                        "A2": "!A"
                    },
                    "project E": {
                        "A1": "!B",
                        "A2": "!B"
                    }
                },
                "preset Y": {
                    "project A": {
                        "A1": "!V",
                        "A2": "!V"
                    },
                    "project B": {
                        "A1": "!A",
                        "A2": "!B"
                    },
                    "project C": {
                        "A1": "!B",
                        "A2": "!B"
                    },
                    "project D": {
                        "A1": "!A",
                        "A2": "!A"
                    },
                    "project E": {
                        "A1": "!B",
                        "A2": "!B"
                    }
                }
            }"""
        )
        self.assertEqual(DiffTableFormatter.run(DiffTableSet(diff), OutputFormat.JSON), expected_formatted_table)

    def test_diff_table_formatter_console_relative(self):
        differ = BenchmarkDiffer(DifferenceStyle.RELATIVE, 4, OutputFormat.CONSOLE)
        diff = differ.run(self.report_before, self.report_after)

        expected_formatted_table = dedent("""
            PRESET X
            |-----------|--------|----|
            |   project |     A1 | A2 |
            |-----------|--------|----|
            | project A | 0.0101 |  0 |
            | project B |     !B | !A |
            | project C |     !A | !A |
            | project D |     !A | !A |
            | project E |     !B | !B |
            |-----------|--------|----|

            PRESET Y
            |-----------|----|----|
            |   project | A1 | A2 |
            |-----------|----|----|
            | project A | !V | !V |
            | project B | !A | !B |
            | project C | !B | !B |
            | project D | !A | !A |
            | project E | !B | !B |
            |-----------|----|----|
        """)
        self.assertEqual(DiffTableFormatter.run(DiffTableSet(diff), OutputFormat.CONSOLE), expected_formatted_table)

    def test_diff_table_formatter_markdown_humanized(self):
        differ = BenchmarkDiffer(DifferenceStyle.HUMANIZED, 4, OutputFormat.MARKDOWN)
        diff = differ.run(self.report_before, self.report_after)

        expected_formatted_table = dedent("""
            ### `preset X`
            |   project |             A1 |   A2 |
            |:---------:|---------------:|-----:|
            | project A | **`+1.01% ❌`** | `0%` |
            | project B |           `!B` | `!A` |
            | project C |           `!A` | `!A` |
            | project D |           `!A` | `!A` |
            | project E |           `!B` | `!B` |

            ### `preset Y`
            |   project |   A1 |   A2 |
            |:---------:|-----:|-----:|
            | project A | `!V` | `!V` |
            | project B | `!A` | `!B` |
            | project C | `!B` | `!B` |
            | project D | `!A` | `!A` |
            | project E | `!B` | `!B` |


            `!V` = version mismatch
            `!B` = no value in the "before" version
            `!A` = no value in the "after" version
            `!T` = one or both values were not numeric and could not be compared
            `-0` = very small negative value rounded to zero
            `+0` = very small positive value rounded to zero

        """)
        self.assertEqual(DiffTableFormatter.run(DiffTableSet(diff), OutputFormat.MARKDOWN), expected_formatted_table)
