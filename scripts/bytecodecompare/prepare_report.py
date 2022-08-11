#!/usr/bin/env python3

import sys
import subprocess
import json
import re
from argparse import ArgumentParser
from dataclasses import dataclass
from enum import Enum
from glob import glob
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import List, Optional, Tuple, Union


CONTRACT_SEPARATOR_PATTERN = re.compile(
    r'^ *======= +(?:(?P<file_name>.+) *:)? *(?P<contract_name>[^:]+) +======= *$',
    re.MULTILINE
)
BYTECODE_REGEX = re.compile(r'^ *Binary: *\n(?P<bytecode>.*[0-9a-f$_]+.*)$', re.MULTILINE)
METADATA_REGEX = re.compile(r'^ *Metadata: *\n *(?P<metadata>\{.*\}) *$', re.MULTILINE)


class CompilerInterface(Enum):
    CLI = 'cli'
    STANDARD_JSON = 'standard-json'


class SMTUse(Enum):
    PRESERVE = 'preserve'
    DISABLE = 'disable'
    STRIP_PRAGMAS = 'strip-pragmas'


@dataclass(frozen=True)
class ContractReport:
    contract_name: str
    file_name: Optional[Path]
    bytecode: Optional[str]
    metadata: Optional[str]


@dataclass
class FileReport:
    file_name: Path
    contract_reports: Optional[List[ContractReport]]

    def format_report(self) -> str:
        report = ""

        if self.contract_reports is None:
            return f"{self.file_name.as_posix()}: <ERROR>\n"

        for contract_report in self.contract_reports:
            bytecode = contract_report.bytecode if contract_report.bytecode is not None else '<NO BYTECODE>'
            metadata = contract_report.metadata if contract_report.metadata is not None else '<NO METADATA>'

            # NOTE: Ignoring contract_report.file_name because it should always be either the same
            # as self.file_name (for Standard JSON) or just the '<stdin>' placeholder (for CLI).
            report += f"{self.file_name.as_posix()}:{contract_report.contract_name} {bytecode}\n"
            report += f"{self.file_name.as_posix()}:{contract_report.contract_name} {metadata}\n"

        return report

    def format_summary(self, verbose: bool) -> str:
        error = (self.contract_reports is None)
        contract_reports = self.contract_reports if self.contract_reports is not None else []
        no_bytecode = any(bytecode is None for bytecode in contract_reports)
        no_metadata = any(metadata is None for metadata in contract_reports)

        if verbose:
            flags = ('E' if error else ' ') + ('B' if no_bytecode else ' ') + ('M' if no_metadata else ' ')
            contract_count = '?' if self.contract_reports is None else str(len(self.contract_reports))
            return f"{contract_count} {flags} {self.file_name}"
        else:
            if error:
                return 'E'
            if no_bytecode:
                return 'B'
            if no_metadata:
                return 'M'

            return '.'


@dataclass
class Statistics:
    file_count: int = 0
    contract_count: int = 0
    error_count: int = 0
    missing_bytecode_count: int = 0
    missing_metadata_count: int = 0

    def aggregate(self, report: FileReport):
        contract_reports = report.contract_reports if report.contract_reports is not None else []

        self.file_count += 1
        self.contract_count += len(contract_reports)
        self.error_count += (1 if report.contract_reports is None else 0)
        self.missing_bytecode_count += sum(1 for c in contract_reports if c.bytecode is None)
        self.missing_metadata_count += sum(1 for c in contract_reports if c.metadata is None)

    def __str__(self) -> str:
        contract_count = str(self.contract_count) + ('+' if self.error_count > 0 else '')
        return (
            f"test cases: {self.file_count}, "
            f"contracts: {contract_count}, "
            f"errors: {self.error_count}, "
            f"missing bytecode: {self.missing_bytecode_count}, "
            f"missing metadata: {self.missing_metadata_count}"
        )


def load_source(path: Union[Path, str], smt_use: SMTUse) -> str:
    # NOTE: newline='' disables newline conversion.
    # We want the file exactly as is because changing even a single byte in the source affects metadata.
    with open(path, mode='r', encoding='utf8', newline='') as source_file:
        file_content = source_file.read()

    if smt_use == SMTUse.STRIP_PRAGMAS:
        return file_content.replace('pragma experimental SMTChecker;', '', 1)

    return file_content


def clean_string(value: Optional[str]) -> Optional[str]:
    value = value.strip() if value is not None else None
    return value if value != '' else None


def parse_standard_json_output(source_file_name: Path, standard_json_output: str) -> FileReport:
    decoded_json_output = json.loads(standard_json_output.strip())

    # JSON interface still returns contract metadata in case of an internal compiler error while
    # CLI interface does not. To make reports comparable we must force this case to be detected as
    # an error in both cases.
    internal_compiler_error = any(
        error['type'] in ['UnimplementedFeatureError', 'CompilerError', 'CodeGenerationError']
        for error in decoded_json_output.get('errors', {})
    )

    if (
        'contracts' not in decoded_json_output or
        len(decoded_json_output['contracts']) == 0 or
        all(len(file_results) == 0 for file_name, file_results in decoded_json_output['contracts'].items()) or
        internal_compiler_error
    ):
        return FileReport(file_name=source_file_name, contract_reports=None)

    file_report = FileReport(file_name=source_file_name, contract_reports=[])
    for file_name, file_results in sorted(decoded_json_output['contracts'].items()):
        for contract_name, contract_results in sorted(file_results.items()):
            if file_report.contract_reports is None:
                raise AssertionError
            file_report.contract_reports.append(ContractReport(
                contract_name=contract_name,
                file_name=Path(file_name),
                bytecode=clean_string(contract_results.get('evm', {}).get('bytecode', {}).get('object')),
                metadata=clean_string(contract_results.get('metadata')),
            ))

    return file_report


def parse_cli_output(source_file_name: Path, cli_output: str) -> FileReport:
    # re.split() returns a list containing the text between pattern occurrences but also inserts the
    # content of matched groups in between. It also never omits the empty elements so the number of
    # list items is predictable (3 per match + the text before the first match)
    output_segments = re.split(CONTRACT_SEPARATOR_PATTERN, cli_output)
    if len(output_segments) % 3 != 1:
        raise AssertionError

    if len(output_segments) == 1:
        return FileReport(file_name=source_file_name, contract_reports=None)

    file_report = FileReport(file_name=source_file_name, contract_reports=[])
    for file_name, contract_name, contract_output in zip(output_segments[1::3], output_segments[2::3], output_segments[3::3]):
        bytecode_match = re.search(BYTECODE_REGEX, contract_output)
        metadata_match = re.search(METADATA_REGEX, contract_output)

        if file_report.contract_reports is None:
            raise AssertionError
        file_report.contract_reports.append(ContractReport(
            contract_name=contract_name.strip(),
            file_name=Path(file_name.strip()) if file_name is not None else None,
            bytecode=clean_string(bytecode_match['bytecode'] if bytecode_match is not None else None),
            metadata=clean_string(metadata_match['metadata'] if metadata_match is not None else None),
        ))

    return file_report


def prepare_compiler_input(
    compiler_path: Path,
    source_file_name: Path,
    optimize: bool,
    force_no_optimize_yul: bool,
    interface: CompilerInterface,
    smt_use: SMTUse,
    metadata_option_supported: bool,
) -> Tuple[List[str], str]:

    if interface == CompilerInterface.STANDARD_JSON:
        json_input: dict = {
            'language': 'Solidity',
            'sources': {
                str(source_file_name): {'content': load_source(source_file_name, smt_use)}
            },
            'settings': {
                'optimizer': {'enabled': optimize},
                'outputSelection': {'*': {'*': ['evm.bytecode.object', 'metadata']}},
            }
        }

        if smt_use == SMTUse.DISABLE:
            json_input['settings']['modelChecker'] = {'engine': 'none'}

        command_line = [str(compiler_path), '--standard-json']
        compiler_input = json.dumps(json_input)
    else:
        if interface != CompilerInterface.CLI:
            raise AssertionError

        compiler_options = [str(source_file_name), '--bin']
        if metadata_option_supported:
            compiler_options.append('--metadata')
        if optimize:
            compiler_options.append('--optimize')
        elif force_no_optimize_yul:
            compiler_options.append('--no-optimize-yul')
        if smt_use == SMTUse.DISABLE:
            compiler_options += ['--model-checker-engine', 'none']

        command_line = [str(compiler_path)] + compiler_options
        compiler_input = load_source(source_file_name, smt_use)

    return (command_line, compiler_input)


def detect_metadata_cli_option_support(compiler_path: Path):
    process = subprocess.run(
        [str(compiler_path.absolute()), '--metadata', '-'],
        input="contract C {}",
        encoding='utf8',
        capture_output=True,
        check=False,
    )

    negative_response = "unrecognised option '--metadata'".strip()
    if (process.returncode == 0) != (process.stderr.strip() != negative_response):
        # If the error is other than expected or there's an error message but no error, don't try
        # to guess. Just fail.
        print(
            f"Compiler exit code: {process.returncode}\n"
            f"Compiler output:\n{process.stderr}\n",
            file=sys.stderr
        )
        raise Exception("Failed to determine if the compiler supports the --metadata option.")

    return process.returncode == 0


def run_compiler(
    compiler_path: Path,
    source_file_name: Path,
    optimize: bool,
    force_no_optimize_yul: bool,
    interface: CompilerInterface,
    smt_use: SMTUse,
    metadata_option_supported: bool,
    tmp_dir: Path,
    exit_on_error: bool,
) -> FileReport:

    if interface == CompilerInterface.STANDARD_JSON:
        (command_line, compiler_input) = prepare_compiler_input(
            compiler_path,
            Path(source_file_name.name),
            optimize,
            force_no_optimize_yul,
            interface,
            smt_use,
            metadata_option_supported,
        )

        process = subprocess.run(
            command_line,
            input=compiler_input,
            encoding='utf8',
            capture_output=True,
            check=exit_on_error,
        )

        return parse_standard_json_output(Path(source_file_name), process.stdout)
    else:
        if interface != CompilerInterface.CLI:
            raise AssertionError
        if tmp_dir is None:
            raise AssertionError

        (command_line, compiler_input) = prepare_compiler_input(
            compiler_path.absolute(),
            Path(source_file_name.name),
            optimize,
            force_no_optimize_yul,
            interface,
            smt_use,
            metadata_option_supported,
        )

        # Create a copy that we can use directly with the CLI interface
        modified_source_path = tmp_dir / source_file_name.name
        # NOTE: newline='' disables newline conversion.
        # We want the file exactly as is because changing even a single byte in the source affects metadata.
        with open(modified_source_path, 'w', encoding='utf8', newline='') as modified_source_file:
            modified_source_file.write(compiler_input)

        process = subprocess.run(
            command_line,
            cwd=tmp_dir,
            encoding='utf8',
            capture_output=True,
            check=exit_on_error,
        )

        return parse_cli_output(Path(source_file_name), process.stdout)


def generate_report(
    source_file_names: List[str],
    compiler_path: Path,
    interface: CompilerInterface,
    smt_use: SMTUse,
    force_no_optimize_yul: bool,
    report_file_path: Path,
    verbose: bool,
    exit_on_error: bool,
):
    statistics = Statistics()
    metadata_option_supported = detect_metadata_cli_option_support(compiler_path)

    try:
        with open(report_file_path, mode='w', encoding='utf8', newline='\n') as report_file:
            for optimize in [False, True]:
                with TemporaryDirectory(prefix='prepare_report-') as tmp_dir:
                    for source_file_name in sorted(source_file_names):
                        try:
                            report = run_compiler(
                                compiler_path,
                                Path(source_file_name),
                                optimize,
                                force_no_optimize_yul,
                                interface,
                                smt_use,
                                metadata_option_supported,
                                Path(tmp_dir),
                                exit_on_error,
                            )

                            statistics.aggregate(report)
                            print(report.format_summary(verbose), end=('\n' if verbose else ''), flush=True)

                            report_file.write(report.format_report())
                        except subprocess.CalledProcessError as exception:
                            print(
                                f"\n\nInterrupted by an exception while processing file "
                                f"'{source_file_name}' with optimize={optimize}\n\n"
                                f"COMPILER STDOUT:\n{exception.stdout}\n"
                                f"COMPILER STDERR:\n{exception.stderr}\n",
                                file=sys.stderr
                            )
                            raise
                        except:
                            print(
                                f"\n\nInterrupted by an exception while processing file "
                                f"'{source_file_name}' with optimize={optimize}\n",
                                file=sys.stderr
                            )
                            raise
    finally:
        print('\n', statistics, '\n', sep='')


def commandline_parser() -> ArgumentParser:
    script_description = (
        "Generates a report listing bytecode and metadata obtained by compiling all the "
        "*.sol files found in the current working directory using the provided binary."
    )

    parser = ArgumentParser(description=script_description)
    parser.add_argument(dest='compiler_path', help="Solidity compiler executable")
    parser.add_argument(
        '--interface',
        dest='interface',
        default=CompilerInterface.STANDARD_JSON.value,
        choices=[c.value for c in CompilerInterface],
        help="Compiler interface to use.",
    )
    parser.add_argument(
        '--smt-use',
        dest='smt_use',
        default=SMTUse.DISABLE.value,
        choices=[s.value for s in SMTUse],
        help="What to do about contracts that use the experimental SMT checker."
    )
    parser.add_argument(
        '--force-no-optimize-yul',
        dest='force_no_optimize_yul',
        default=False,
        action='store_true',
        help="Explicitly disable Yul optimizer in CLI runs without optimization to work around a bug in solc 0.6.0 and 0.6.1."
    )
    parser.add_argument('--report-file', dest='report_file', default='report.txt', help="The file to write the report to.")
    parser.add_argument('--verbose', dest='verbose', default=False, action='store_true', help="More verbose output.")
    parser.add_argument(
        '--exit-on-error',
        dest='exit_on_error',
        default=False,
        action='store_true',
        help="Immediately exit and print compiler output if the compiler exits with an error.",
    )
    return parser


if __name__ == "__main__":
    options = commandline_parser().parse_args()
    generate_report(
        glob("*.sol"),
        Path(options.compiler_path),
        CompilerInterface(options.interface),
        SMTUse(options.smt_use),
        options.force_no_optimize_yul,
        Path(options.report_file),
        options.verbose,
        options.exit_on_error,
    )
