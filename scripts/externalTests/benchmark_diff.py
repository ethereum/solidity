#!/usr/bin/env python3

from argparse import ArgumentParser
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from textwrap import dedent
from typing import Any, Mapping, Optional, Set, Sequence, Union
import json
import sys


class DiffMode(Enum):
    IN_PLACE = 'inplace'
    TABLE = 'table'


class DifferenceStyle(Enum):
    ABSOLUTE = 'absolute'
    RELATIVE = 'relative'
    HUMANIZED = 'humanized'


class OutputFormat(Enum):
    JSON = 'json'
    CONSOLE = 'console'
    MARKDOWN = 'markdown'


DEFAULT_RELATIVE_PRECISION = 4

DEFAULT_DIFFERENCE_STYLE = {
    DiffMode.IN_PLACE: DifferenceStyle.ABSOLUTE,
    DiffMode.TABLE: DifferenceStyle.HUMANIZED,
}
assert all(t in DiffMode for t in DEFAULT_DIFFERENCE_STYLE)
assert all(d in DifferenceStyle for d in DEFAULT_DIFFERENCE_STYLE.values())

DEFAULT_OUTPUT_FORMAT = {
    DiffMode.IN_PLACE: OutputFormat.JSON,
    DiffMode.TABLE: OutputFormat.CONSOLE,
}
assert all(m in DiffMode for m in DEFAULT_OUTPUT_FORMAT)
assert all(o in OutputFormat for o in DEFAULT_OUTPUT_FORMAT.values())


class ValidationError(Exception):
    pass


class CommandLineError(ValidationError):
    pass


class BenchmarkDiffer:
    difference_style: DifferenceStyle
    relative_precision: Optional[int]
    output_format: OutputFormat

    def __init__(
        self,
        difference_style: DifferenceStyle,
        relative_precision: Optional[int],
        output_format: OutputFormat,
    ):
        self.difference_style = difference_style
        self.relative_precision = relative_precision
        self.output_format = output_format

    def run(self, before: Any, after: Any) -> Optional[Union[dict, str, int, float]]:
        if not isinstance(before, dict) or not isinstance(after, dict):
            return self._diff_scalars(before, after)

        if before.get('version') != after.get('version'):
            return self._humanize_diff('!V')

        diff = {}
        for key in (set(before) | set(after)) - {'version'}:
            value_diff = self.run(before.get(key), after.get(key))
            if value_diff not in [None, {}]:
                diff[key] = value_diff

        return diff

    def _diff_scalars(self, before: Any, after: Any) -> Optional[Union[str, int, float, dict]]:
        assert not isinstance(before, dict) or not isinstance(after, dict)

        if before is None and after is None:
            return {}
        if before is None:
            return self._humanize_diff('!B')
        if after is None:
            return self._humanize_diff('!A')
        if not isinstance(before, (int, float)) or not isinstance(after, (int, float)):
            return self._humanize_diff('!T')

        number_diff = self._diff_numbers(before, after)
        if self.difference_style != DifferenceStyle.HUMANIZED:
            return number_diff

        return self._humanize_diff(number_diff)

    def _diff_numbers(self, value_before: Union[int, float], value_after: Union[int, float]) -> Union[str, int, float]:
        diff: Union[str, int, float]

        if self.difference_style == DifferenceStyle.ABSOLUTE:
            diff = value_after - value_before
            if isinstance(diff, float) and diff.is_integer():
                diff = int(diff)

            return diff

        if value_before == 0:
            if value_after > 0:
                return '+INF'
            elif value_after < 0:
                return '-INF'
            else:
                return 0

        diff = (value_after - value_before) / abs(value_before)
        if self.relative_precision is not None:
            rounded_diff = round(diff, self.relative_precision)
            if rounded_diff == 0 and diff < 0:
                diff = '-0'
            elif rounded_diff == 0 and diff > 0:
                diff = '+0'
            else:
                diff = rounded_diff

        if isinstance(diff, float) and diff.is_integer():
            diff = int(diff)

        return diff

    def _humanize_diff(self, diff: Union[str, int, float]) -> str:
        def wrap(value: str, symbol: str):
            return f"{symbol}{value}{symbol}"

        markdown = (self.output_format == OutputFormat.MARKDOWN)

        if isinstance(diff, str) and diff.startswith('!'):
            return wrap(diff, '`' if markdown else '')

        value: Union[str, int, float]
        if isinstance(diff, (int, float)):
            value = diff * 100
            if isinstance(value, float) and self.relative_precision is not None:
                # The multiplication can result in new significant digits appearing. We need to reround.
                # NOTE: round() works fine with negative precision.
                value = round(value, self.relative_precision - 2)
                if isinstance(value, float) and value.is_integer():
                    value = int(value)
            suffix = ''
            prefix = ''
            if diff < 0:
                prefix = ''
                if markdown:
                    suffix += ' ✅'
            elif diff > 0:
                prefix = '+'
                if markdown:
                    suffix += ' ❌'
            important = (diff != 0)
        else:
            value = diff
            important = False
            prefix = ''
            suffix = ''

        return wrap(
            wrap(
                f"{prefix}{value}%{suffix}",
                '`' if markdown else ''
            ),
            '**' if important and markdown else ''
        )


@dataclass(frozen=True)
class DiffTable:
    columns: Mapping[str, Sequence[Union[int, float, str]]]


class DiffTableSet:
    table_headers: Sequence[str]
    row_headers: Sequence[str]
    column_headers: Sequence[str]

    # Cells is a nested dict rather than a 3D array so that conversion to JSON is straightforward
    cells: Mapping[str, Mapping[str, Mapping[str, Union[int, float, str]]]] # preset -> project -> attribute

    def __init__(self, diff: dict):
        self.table_headers = sorted(self._find_all_preset_names(diff))
        self.column_headers = sorted(self._find_all_attribute_names(diff))
        self.row_headers = sorted(project for project in diff)

        # All dimensions must have unique values
        assert len(self.table_headers) == len(set(self.table_headers))
        assert len(self.column_headers) == len(set(self.column_headers))
        assert len(self.row_headers) == len(set(self.row_headers))

        self.cells = {
            preset: {
                project: {
                    attribute: self._cell_content(diff, project, preset, attribute)
                    for attribute in self.column_headers
                }
                for project in self.row_headers
            }
            for preset in self.table_headers
        }

    def calculate_row_column_width(self) -> int:
        return max(len(h) for h in self.row_headers)

    def calculate_column_widths(self, table_header: str) -> Sequence[int]:
        assert table_header in self.table_headers

        return [
            max(
                len(column_header),
                max(
                    len(str(self.cells[table_header][row_header][column_header]))
                    for row_header in self.row_headers
                )
            )
            for column_header in self.column_headers
        ]

    @classmethod
    def _find_all_preset_names(cls, diff: dict) -> Set[str]:
        return {
            preset
            for project, project_diff in diff.items()
            if isinstance(project_diff, dict)
            for preset in project_diff
        }

    @classmethod
    def _find_all_attribute_names(cls, diff: dict) -> Set[str]:
        return {
            attribute
            for project, project_diff in diff.items()
            if isinstance(project_diff, dict)
            for preset, preset_diff in project_diff.items()
            if isinstance(preset_diff, dict)
            for attribute in preset_diff
        }

    @classmethod
    def _cell_content(cls, diff: dict, project: str, preset: str, attribute: str) -> str:
        assert project in diff

        if isinstance(diff[project], str):
            return diff[project]
        if preset not in diff[project]:
            return ''
        if isinstance(diff[project][preset], str):
            return diff[project][preset]
        if attribute not in diff[project][preset]:
            return ''

        return diff[project][preset][attribute]


class DiffTableFormatter:
    LEGEND = dedent("""
        `!V` = version mismatch
        `!B` = no value in the "before" version
        `!A` = no value in the "after" version
        `!T` = one or both values were not numeric and could not be compared
        `-0` = very small negative value rounded to zero
        `+0` = very small positive value rounded to zero
    """)

    @classmethod
    def run(cls, diff_table_set: DiffTableSet, output_format: OutputFormat):
        if output_format == OutputFormat.JSON:
            return json.dumps(diff_table_set.cells, indent=4, sort_keys=True)
        else:
            assert output_format in {OutputFormat.CONSOLE, OutputFormat.MARKDOWN}

            output = ''
            for table_header in diff_table_set.table_headers:
                column_widths = ([
                    diff_table_set.calculate_row_column_width(),
                    *diff_table_set.calculate_column_widths(table_header)
                ])

                if output_format == OutputFormat.MARKDOWN:
                    output += f'\n### `{table_header}`\n'
                else:
                    output += f'\n{table_header.upper()}\n'

                if output_format == OutputFormat.CONSOLE:
                    output += cls._format_separator_row(column_widths, output_format) + '\n'
                output += cls._format_data_row(['project', *diff_table_set.column_headers], column_widths) + '\n'
                output += cls._format_separator_row(column_widths, output_format) + '\n'

                for row_header in diff_table_set.row_headers:
                    row = [
                        diff_table_set.cells[table_header][row_header][column_header]
                        for column_header in diff_table_set.column_headers
                    ]
                    output += cls._format_data_row([row_header, *row], column_widths) + '\n'

                if output_format == OutputFormat.CONSOLE:
                    output += cls._format_separator_row(column_widths, output_format) + '\n'

            if output_format == OutputFormat.MARKDOWN:
                output += f'\n{cls.LEGEND}\n'
            return output

    @classmethod
    def _format_separator_row(cls, widths: Sequence[int], output_format: OutputFormat):
        assert output_format in {OutputFormat.CONSOLE, OutputFormat.MARKDOWN}

        if output_format == OutputFormat.MARKDOWN:
            return '|:' + ':|-'.join('-' * width for width in widths) + ':|'
        else:
            return '|-' + '-|-'.join('-' * width for width in widths) + '-|'

    @classmethod
    def _format_data_row(cls, cells: Sequence[Union[int, float, str]], widths: Sequence[int]):
        assert len(cells) == len(widths)

        return '| ' + ' | '.join(str(cell).rjust(width) for cell, width in zip(cells, widths)) + ' |'


@dataclass(frozen=True)
class CommandLineOptions:
    diff_mode: DiffMode
    report_before: Path
    report_after: Path
    difference_style: DifferenceStyle
    relative_precision: int
    output_format: OutputFormat


def process_commandline() -> CommandLineOptions:
    script_description = (
        "Compares summarized benchmark reports and outputs JSON with the same structure but listing only differences. "
        "Can also print the output as markdown table and format the values to make differences stand out more."
    )

    parser = ArgumentParser(description=script_description)
    parser.add_argument(
        dest='diff_mode',
        choices=[m.value for m in DiffMode],
        help=(
            "Diff mode: "
            f"'{DiffMode.IN_PLACE.value}' preserves input JSON structure and replace values with differences; "
            f"'{DiffMode.TABLE.value}' creates a table assuming 3-level project/preset/attribute structure."
        )
    )
    parser.add_argument(dest='report_before', help="Path to a JSON file containing original benchmark results.")
    parser.add_argument(dest='report_after', help="Path to a JSON file containing new benchmark results.")
    parser.add_argument(
        '--style',
        dest='difference_style',
        choices=[s.value for s in DifferenceStyle],
        help=(
            "How to present numeric differences: "
            f"'{DifferenceStyle.ABSOLUTE.value}' subtracts new from original; "
            f"'{DifferenceStyle.RELATIVE.value}' also divides by the original; "
            f"'{DifferenceStyle.HUMANIZED.value}' is like relative but value is a percentage and "
            "positive/negative changes are emphasized. "
            f"(default: '{DEFAULT_DIFFERENCE_STYLE[DiffMode.IN_PLACE]}' in '{DiffMode.IN_PLACE.value}' mode, "
            f"'{DEFAULT_DIFFERENCE_STYLE[DiffMode.TABLE]}' in '{DiffMode.TABLE.value}' mode)"
        )
    )
    # NOTE: Negative values are valid for precision. round() handles them in a sensible way.
    parser.add_argument(
        '--precision',
        dest='relative_precision',
        type=int,
        default=DEFAULT_RELATIVE_PRECISION,
        help=(
            "Number of significant digits for relative differences. "
            f"Note that with --style={DifferenceStyle.HUMANIZED.value} the rounding is applied "
            "**before** converting the value to a percentage so you need to add 2. "
            f"Has no effect when used together with --style={DifferenceStyle.ABSOLUTE.value}. "
            f"(default: {DEFAULT_RELATIVE_PRECISION})"
        )
    )
    parser.add_argument(
        '--output-format',
        dest='output_format',
        choices=[o.value for o in OutputFormat],
        help=(
            "The format to use for the diff: "
            f"'{OutputFormat.JSON.value}' is raw JSON; "
            f"'{OutputFormat.CONSOLE.value}' is a table with human-readable values that will look good in the console output. "
            f"'{OutputFormat.MARKDOWN.value}' is similar '{OutputFormat.CONSOLE.value}' but adjusted to "
            "render as proper markdown and with extra elements (legend, emoji to make non-zero values stand out more, etc)."
            f"(default: '{DEFAULT_OUTPUT_FORMAT[DiffMode.IN_PLACE]}' in '{DiffMode.IN_PLACE.value}' mode, "
            f"'{DEFAULT_OUTPUT_FORMAT[DiffMode.TABLE]}' in '{DiffMode.TABLE.value}' mode)"
        )
    )

    options = parser.parse_args()

    if options.difference_style is not None:
        difference_style = DifferenceStyle(options.difference_style)
    else:
        difference_style = DEFAULT_DIFFERENCE_STYLE[DiffMode(options.diff_mode)]

    if options.output_format is not None:
        output_format = OutputFormat(options.output_format)
    else:
        output_format = DEFAULT_OUTPUT_FORMAT[DiffMode(options.diff_mode)]

    processed_options = CommandLineOptions(
        diff_mode=DiffMode(options.diff_mode),
        report_before=Path(options.report_before),
        report_after=Path(options.report_after),
        difference_style=difference_style,
        relative_precision=options.relative_precision,
        output_format=output_format,
    )

    if processed_options.diff_mode == DiffMode.IN_PLACE and processed_options.output_format != OutputFormat.JSON:
        raise CommandLineError(
            f"Only the '{OutputFormat.JSON.value}' output format is supported in the '{DiffMode.IN_PLACE.value}' mode."
        )

    return processed_options


def main():
    try:
        options = process_commandline()

        differ = BenchmarkDiffer(options.difference_style, options.relative_precision, options.output_format)
        diff = differ.run(
            json.loads(options.report_before.read_text('utf-8')),
            json.loads(options.report_after.read_text('utf-8')),
        )

        if options.diff_mode == DiffMode.IN_PLACE:
            print(json.dumps(diff, indent=4, sort_keys=True))
        else:
            assert options.diff_mode == DiffMode.TABLE
            print(DiffTableFormatter.run(DiffTableSet(diff), options.output_format))

        return 0
    except CommandLineError as exception:
        print(f"ERROR: {exception}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    sys.exit(main())
