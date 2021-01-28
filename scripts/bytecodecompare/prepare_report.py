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


CONTRACT_SEPARATOR_PATTERN = re.compile(r'^======= (?P<file_name>.+):(?P<contract_name>[^:]+) =======$', re.MULTILINE)
BYTECODE_REGEX = re.compile(r'^Binary:\n(?P<bytecode>.*)$', re.MULTILINE)
METADATA_REGEX = re.compile(r'^Metadata:\n(?P<metadata>\{.*\})$', re.MULTILINE)


class CompilerInterface(Enum):
    CLI = 'cli'
    STANDARD_JSON = 'standard-json'


@dataclass(frozen=True)
class ContractReport:
    contract_name: str
    file_name: Path
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


def load_source(path: Union[Path, str]) -> str:
    # NOTE: newline='' disables newline conversion.
    # We want the file exactly as is because changing even a single byte in the source affects metadata.
    with open(path, mode='r', encoding='utf8', newline='') as source_file:
        file_content = source_file.read()

    return file_content


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
            assert file_report.contract_reports is not None
            file_report.contract_reports.append(ContractReport(
                contract_name=contract_name,
                file_name=Path(file_name),
                bytecode=contract_results.get('evm', {}).get('bytecode', {}).get('object'),
                metadata=contract_results.get('metadata'),
            ))

    return file_report


def parse_cli_output(source_file_name: Path, cli_output: str) -> FileReport:
    # re.split() returns a list containing the text between pattern occurrences but also inserts the
    # content of matched groups in between. It also never omits the empty elements so the number of
    # list items is predictable (3 per match + the text before the first match)
    output_segments = re.split(CONTRACT_SEPARATOR_PATTERN, cli_output)
    assert len(output_segments) % 3 == 1

    if len(output_segments) == 1:
        return FileReport(file_name=source_file_name, contract_reports=None)

    file_report = FileReport(file_name=source_file_name, contract_reports=[])
    for file_name, contract_name, contract_output in zip(output_segments[1::3], output_segments[2::3], output_segments[3::3]):
        bytecode_match = re.search(BYTECODE_REGEX, contract_output)
        metadata_match = re.search(METADATA_REGEX, contract_output)

        assert file_report.contract_reports is not None
        file_report.contract_reports.append(ContractReport(
            contract_name=contract_name,
            file_name=Path(file_name),
            bytecode=bytecode_match['bytecode'] if bytecode_match is not None else None,
            metadata=metadata_match['metadata'] if metadata_match is not None else None,
        ))

    return file_report


def prepare_compiler_input(
    compiler_path: Path,
    source_file_name: Path,
    optimize: bool,
    interface: CompilerInterface
) -> Tuple[List[str], str]:

    if interface == CompilerInterface.STANDARD_JSON:
        json_input: dict = {
            'language': 'Solidity',
            'sources': {
                str(source_file_name): {'content': load_source(source_file_name)}
            },
            'settings': {
                'optimizer': {'enabled': optimize},
                'outputSelection': {'*': {'*': ['evm.bytecode.object', 'metadata']}},
                'modelChecker': {'engine': 'none'},
            }
        }

        command_line = [str(compiler_path), '--standard-json']
        compiler_input = json.dumps(json_input)
    else:
        assert interface == CompilerInterface.CLI

        compiler_options = [str(source_file_name), '--bin', '--metadata', '--model-checker-engine', 'none']
        if optimize:
            compiler_options.append('--optimize')

        command_line = [str(compiler_path)] + compiler_options
        compiler_input = load_source(source_file_name)

    return (command_line, compiler_input)


def run_compiler(
    compiler_path: Path,
    source_file_name: Path,
    optimize: bool,
    interface: CompilerInterface,
    tmp_dir: Path,
) -> FileReport:

    if interface == CompilerInterface.STANDARD_JSON:
        (command_line, compiler_input) = prepare_compiler_input(
            compiler_path,
            Path(source_file_name.name),
            optimize,
            interface
        )

        process = subprocess.run(
            command_line,
            input=compiler_input,
            encoding='utf8',
            capture_output=True,
            check=False,
        )

        return parse_standard_json_output(Path(source_file_name), process.stdout)
    else:
        assert interface == CompilerInterface.CLI
        assert tmp_dir is not None

        (command_line, compiler_input) = prepare_compiler_input(
            compiler_path.absolute(),
            Path(source_file_name.name),
            optimize,
            interface
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
            check=False,
        )

        return parse_cli_output(Path(source_file_name), process.stdout)


def generate_report(source_file_names: List[str], compiler_path: Path, interface: CompilerInterface):
    with open('report.txt', mode='w', encoding='utf8', newline='\n') as report_file:
        for optimize in [False, True]:
            with TemporaryDirectory(prefix='prepare_report-') as tmp_dir:
                for source_file_name in sorted(source_file_names):
                    try:
                        report = run_compiler(compiler_path, Path(source_file_name), optimize, interface, Path(tmp_dir))
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
        help="Compiler interface to use."
    )
    return parser;


if __name__ == "__main__":
    options = commandline_parser().parse_args()
    generate_report(
        glob("*.sol"),
        Path(options.compiler_path),
        CompilerInterface(options.interface),
    )
