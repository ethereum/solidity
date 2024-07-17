#!/usr/bin/env python3
# coding=utf-8

from dataclasses import asdict, dataclass, field
from typing import Dict, Optional, Tuple
import json
import re
import sys

REPORT_HEADER_REGEX = re.compile(r'''
    ^[|\s]+ Solc[ ]version:\s*(?P<solc_version>[\w\d.]+)
    [|\s]+ Optimizer[ ]enabled:\s*(?P<optimize>[\w]+)
    [|\s]+ Runs:\s*(?P<runs>[\d]+)
    [|\s]+ Block[ ]limit:\s*(?P<block_limit>[\d]+)\s*gas
    [|\s]+$
''', re.VERBOSE)
METHOD_HEADER_REGEX = re.compile(r'^[|\s]+Methods[|\s]+$')
METHOD_COLUMN_HEADERS_REGEX = re.compile(r'''
    ^[|\s]+ Contract
    [|\s]+ Method
    [|\s]+ Min
    [|\s]+ Max
    [|\s]+ Avg
    [|\s]+ \#[ ]calls
    [|\s]+ \w+[ ]\(avg\)
    [|\s]+$
''', re.VERBOSE)
METHOD_ROW_REGEX = re.compile(r'''
    ^[|\s]+ (?P<contract>[^|]+)
    [|\s]+ (?P<method>[^|]+)
    [|\s]+ (?P<min>[^|]+)
    [|\s]+ (?P<max>[^|]+)
    [|\s]+ (?P<avg>[^|]+)
    [|\s]+ (?P<call_count>[^|]+)
    [|\s]+ (?P<eur_avg>[^|]+)
    [|\s]+$
''', re.VERBOSE)
FRAME_REGEX = re.compile(r'^[-|\s]+$')
DEPLOYMENT_HEADER_REGEX = re.compile(r'^[|\s]+Deployments[|\s]+% of limit[|\s]+$')
DEPLOYMENT_ROW_REGEX = re.compile(r'''
    ^[|\s]+ (?P<contract>[^|]+)
    [|\s]+ (?P<min>[^|]+)
    [|\s]+ (?P<max>[^|]+)
    [|\s]+ (?P<avg>[^|]+)
    [|\s]+ (?P<percent_of_limit>[^|]+)\s*%
    [|\s]+ (?P<eur_avg>[^|]+)
    [|\s]+$
''', re.VERBOSE)


class ReportError(Exception):
    pass

class ReportValidationError(ReportError):
    pass

class ReportParsingError(Exception):
    def __init__(self, message: str, line: str, line_number: int):
        # pylint: disable=useless-super-delegation  # It's not useless, it adds type annotations.
        super().__init__(message, line, line_number)

    def __str__(self):
        return f"Parsing error on line {self.args[2] + 1}: {self.args[0]}\n{self.args[1]}"


@dataclass(frozen=True)
class MethodGasReport:
    min_gas: int
    max_gas: int
    avg_gas: int
    call_count: int
    total_gas: int = field(init=False)

    def __post_init__(self):
        object.__setattr__(self, 'total_gas', self.avg_gas * self.call_count)


@dataclass(frozen=True)
class ContractGasReport:
    min_deployment_gas: Optional[int]
    max_deployment_gas: Optional[int]
    avg_deployment_gas: Optional[int]
    methods: Optional[Dict[str, MethodGasReport]]
    total_method_gas: int = field(init=False, default=0)

    def __post_init__(self):
        if self.methods is not None:
            object.__setattr__(self, 'total_method_gas', sum(method.total_gas for method in self.methods.values()))


@dataclass(frozen=True)
class GasReport:
    solc_version: str
    optimize: bool
    runs: int
    block_limit: int
    contracts: Dict[str, ContractGasReport]
    total_method_gas: int = field(init=False)
    total_deployment_gas: int = field(init=False)

    def __post_init__(self):
        object.__setattr__(self, 'total_method_gas', sum(
            total_method_gas
            for total_method_gas in (contract.total_method_gas for contract in self.contracts.values())
            if total_method_gas is not None
        ))
        object.__setattr__(self, 'total_deployment_gas', sum(
            contract.avg_deployment_gas
            for contract in self.contracts.values()
            if contract.avg_deployment_gas is not None
        ))

    def to_json(self):
        return json.dumps(asdict(self), indent=4, sort_keys=True)


def parse_bool(input_string: str) -> bool:
    if input_string == 'true':
        return True
    elif input_string == 'false':
        return True
    else:
        raise ValueError(f"Invalid boolean value: '{input_string}'")


def parse_optional_int(input_string: str, default: Optional[int] = None) -> Optional[int]:
    if input_string.strip() == '-':
        return default

    return int(input_string)


def parse_report_header(line: str) -> Optional[dict]:
    match = REPORT_HEADER_REGEX.match(line)
    if match is None:
        return None

    return {
        'solc_version': match.group('solc_version'),
        'optimize': parse_bool(match.group('optimize')),
        'runs': int(match.group('runs')),
        'block_limit': int(match.group('block_limit')),
    }


def parse_method_row(line: str, line_number: int) -> Optional[Tuple[str, str, MethodGasReport]]:
    match = METHOD_ROW_REGEX.match(line)
    if match is None:
        raise ReportParsingError("Expected a table row with method details.", line, line_number)

    avg_gas = parse_optional_int(match['avg'])
    call_count = int(match['call_count'])

    if avg_gas is None and call_count == 0:
        # No calls, no gas values. Uninteresting. Skip the row.
        return None

    return (
        match['contract'].strip(),
        match['method'].strip(),
        MethodGasReport(
            min_gas=parse_optional_int(match['min'], avg_gas),
            max_gas=parse_optional_int(match['max'], avg_gas),
            avg_gas=avg_gas,
            call_count=call_count,
        )
    )


def parse_deployment_row(line: str, line_number: int) -> Tuple[str, int, int, int]:
    match = DEPLOYMENT_ROW_REGEX.match(line)
    if match is None:
        raise ReportParsingError("Expected a table row with deployment details.", line, line_number)

    return (
        match['contract'].strip(),
        parse_optional_int(match['min'].strip()),
        parse_optional_int(match['max'].strip()),
        int(match['avg'].strip()),
    )


def preprocess_unicode_frames(input_string: str) -> str:
    # The report has a mix of normal pipe chars and its unicode variant.
    # Let's just replace all frame chars with normal pipes for easier parsing.
    return input_string.replace('\u2502', '|').replace('·', '|')


def parse_report(rst_report: str) -> GasReport:
    report_params = None
    methods_by_contract = {}
    deployment_costs = {}
    expected_row_type = None

    for line_number, line in enumerate(preprocess_unicode_frames(rst_report).splitlines()):
        try:
            if (
                line.strip() == "" or
                FRAME_REGEX.match(line) is not None or
                METHOD_COLUMN_HEADERS_REGEX.match(line) is not None
            ):
                continue
            if METHOD_HEADER_REGEX.match(line) is not None:
                expected_row_type = 'method'
                continue
            if DEPLOYMENT_HEADER_REGEX.match(line) is not None:
                expected_row_type = 'deployment'
                continue

            new_report_params = parse_report_header(line)
            if new_report_params is not None:
                if report_params is not None:
                    raise ReportParsingError("Duplicate report header.", line, line_number)

                report_params = new_report_params
                continue

            if expected_row_type == 'method':
                parsed_row = parse_method_row(line, line_number)
                if parsed_row is None:
                    continue

                (contract, method, method_report) = parsed_row

                if contract not in methods_by_contract:
                    methods_by_contract[contract] = {}

                if method in methods_by_contract[contract]:
                    # Report must be generated with full signatures for method names to be unambiguous.
                    raise ReportParsingError(f"Duplicate method row for '{contract}.{method}'.", line, line_number)

                methods_by_contract[contract][method] = method_report
            elif expected_row_type == 'deployment':
                (contract, min_gas, max_gas, avg_gas) = parse_deployment_row(line, line_number)

                if contract in deployment_costs:
                    raise ReportParsingError(f"Duplicate contract deployment row for '{contract}'.", line, line_number)

                deployment_costs[contract] = (min_gas, max_gas, avg_gas)
            else:
                assert expected_row_type is None
                raise ReportParsingError("Found data row without a section header.", line, line_number)

        except ValueError as error:
            raise ReportParsingError(error.args[0], line, line_number) from error

    if report_params is None:
        raise ReportValidationError("Report header not found.")

    report_params['contracts'] = {
        contract: ContractGasReport(
            min_deployment_gas=deployment_costs.get(contract, (None, None, None))[0],
            max_deployment_gas=deployment_costs.get(contract, (None, None, None))[1],
            avg_deployment_gas=deployment_costs.get(contract, (None, None, None))[2],
            methods=methods_by_contract.get(contract),
        )
        for contract in methods_by_contract.keys() | deployment_costs.keys()
    }

    return GasReport(**report_params)


if __name__ == "__main__":
    try:
        report = parse_report(sys.stdin.read())
        print(report.to_json())
    except ReportError as exception:
        print(f"{exception}", file=sys.stderr)
        sys.exit(1)
