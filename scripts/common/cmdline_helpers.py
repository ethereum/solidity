import os
import subprocess
from pathlib import Path
from shutil import rmtree
from tempfile import mkdtemp
from textwrap import dedent
from typing import List
from typing import Optional

from bytecodecompare.prepare_report import FileReport
from bytecodecompare.prepare_report import parse_cli_output


DEFAULT_PREAMBLE = dedent("""
    // SPDX-License-Identifier: UNLICENSED
    pragma solidity >=0.0;
""")


def inside_temporary_dir(prefix):
    """
    Creates a temporary directory, enters the directory and executes the function inside it.
    Restores the previous working directory after executing the function.
    """
    def tmp_dir_decorator(fn):
        previous_dir = os.getcwd()
        def f(*args, **kwargs):
            try:
                tmp_dir = mkdtemp(prefix=prefix)
                os.chdir(tmp_dir)
                result = fn(*args, **kwargs)
                rmtree(tmp_dir)
                return result
            finally:
                os.chdir(previous_dir)
        return f
    return tmp_dir_decorator


def solc_bin_report(solc_binary: str, input_files: List[Path], via_ir: bool) -> FileReport:
    """
    Runs the solidity compiler binary
    """

    output = subprocess.check_output(
        [solc_binary, '--bin'] +
        input_files +
        (['--via-ir'] if via_ir else []),
        encoding='utf8',
    )
    return parse_cli_output('', output)


def save_bytecode(bytecode_path: Path, reports: FileReport, contract: Optional[str] = None):
    with open(bytecode_path, 'w', encoding='utf8') as f:
        for report in reports.contract_reports:
            if contract is None or report.contract_name == contract:
                bytecode = report.bytecode if report.bytecode is not None else '<NO BYTECODE>'
                f.write(f'{report.contract_name}: {bytecode}\n')


def add_preamble(source_path: Path, preamble: str = DEFAULT_PREAMBLE):
    for source in source_path.glob('**/*.sol'):
        with open(source, 'r+', encoding='utf8') as f:
            content = f.read()
            f.seek(0, 0)
            f.write(preamble + content)
