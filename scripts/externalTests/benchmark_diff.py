#!/usr/bin/env python3

from argparse import ArgumentParser
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Any, Optional, Union
import json
import sys


class DifferenceStyle(Enum):
    ABSOLUTE = 'absolute'
    RELATIVE = 'relative'
    HUMANIZED = 'humanized'


DEFAULT_RELATIVE_PRECISION = 4
DEFAULT_DIFFERENCE_STYLE = DifferenceStyle.ABSOLUTE


class ValidationError(Exception):
    pass


class CommandLineError(ValidationError):
    pass


class BenchmarkDiffer:
    difference_style: DifferenceStyle
    relative_precision: Optional[int]

    def __init__(
        self,
        difference_style: DifferenceStyle,
        relative_precision: Optional[int],
    ):
        self.difference_style = difference_style
        self.relative_precision = relative_precision

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
        if isinstance(diff, str) and diff.startswith('!'):
            return diff

        value: Union[str, int, float]
        if isinstance(diff, (int, float)):
            value = diff * 100
            if isinstance(value, float) and self.relative_precision is not None:
                # The multiplication can result in new significant digits appearing. We need to reround.
                # NOTE: round() works fine with negative precision.
                value = round(value, self.relative_precision - 2)
                if isinstance(value, float) and value.is_integer():
                    value = int(value)
            prefix = ''
            if diff < 0:
                prefix = ''
            elif diff > 0:
                prefix = '+'
        else:
            value = diff
            prefix = ''

        return f"{prefix}{value}%"


@dataclass(frozen=True)
class CommandLineOptions:
    report_before: Path
    report_after: Path
    difference_style: DifferenceStyle
    relative_precision: int


def process_commandline() -> CommandLineOptions:
    script_description = (
        "Compares summarized benchmark reports and outputs JSON with the same structure but listing only differences."
    )

    parser = ArgumentParser(description=script_description)
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
            f"(default: '{DEFAULT_DIFFERENCE_STYLE}')."
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

    options = parser.parse_args()

    if options.difference_style is not None:
        difference_style = DifferenceStyle(options.difference_style)
    else:
        difference_style = DEFAULT_DIFFERENCE_STYLE

    processed_options = CommandLineOptions(
        report_before=Path(options.report_before),
        report_after=Path(options.report_after),
        difference_style=difference_style,
        relative_precision=options.relative_precision,
    )

    return processed_options


def main():
    try:
        options = process_commandline()

        differ = BenchmarkDiffer(options.difference_style, options.relative_precision)
        diff = differ.run(
            json.loads(options.report_before.read_text('utf-8')),
            json.loads(options.report_after.read_text('utf-8')),
        )

        print(json.dumps(diff, indent=4, sort_keys=True))

        return 0
    except CommandLineError as exception:
        print(f"ERROR: {exception}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    sys.exit(main())
