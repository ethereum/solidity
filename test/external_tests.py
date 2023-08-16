#!/usr/bin/env python3

# ------------------------------------------------------------------------------
# This file is part of solidity.
#
# solidity is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# solidity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with solidity.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2023 solidity contributors.
# ------------------------------------------------------------------------------

from argparse import ArgumentParser, Namespace
import os
from pathlib import Path
import sys
import subprocess

EXTERNAL_TESTS_DIR = Path(__file__).parent / "externalTests"


class ExternalTestNotFound(Exception):
    pass


def detect_external_tests() -> dict:
    return {
        file_path.stem: file_path
        for file_path in Path(EXTERNAL_TESTS_DIR).iterdir()
        if file_path.is_file() and file_path.suffix in (".sh", ".py")
    }


def display_available_external_tests(_):
    print("Available external tests:")
    print(*detect_external_tests().keys())


def run_test_scripts(solc_binary_type: str, solc_binary_path: Path, tests: dict):
    for test_name, test_script_path in tests.items():
        print(f"Running {test_name} external test...")
        subprocess.run(
            [test_script_path, solc_binary_type, solc_binary_path],
            check=True
        )


def run_external_tests(args: dict):
    solc_binary_type = args["solc_binary_type"]
    solc_binary_path = args["solc_binary_path"]

    all_test_scripts = detect_external_tests()
    selected_tests = args["selected_tests"]
    if args["run_all"]:
        assert len(selected_tests) == 0
        run_test_scripts(solc_binary_type, solc_binary_path, all_test_scripts)
        return

    if len(selected_tests) == 0:
        raise ExternalTestNotFound(
            "External test was not selected. Please use --run or --run-all option"
        )

    unrecognized_tests = set(selected_tests) - set(all_test_scripts.keys())
    if unrecognized_tests != set():
        raise ExternalTestNotFound(
            f"External test(s) not found: {', '.join(unrecognized_tests)}"
        )
    run_test_scripts(
        solc_binary_type,
        solc_binary_path,
        {k: all_test_scripts[k] for k in selected_tests},
    )


def parse_commandline() -> Namespace:
    script_description = "Script to run external Solidity tests."

    parser = ArgumentParser(description=script_description)
    subparser = parser.add_subparsers()
    list_command = subparser.add_parser(
        "list",
        help="List all available external tests.",
    )
    list_command.set_defaults(cmd=display_available_external_tests)

    run_command = subparser.add_parser(
        "test",
        help="Run external tests.",
    )
    run_command.set_defaults(cmd=run_external_tests)

    run_command.add_argument(
        "--solc-binary-type",
        dest="solc_binary_type",
        type=str,
        required=True,
        choices=["native", "solcjs"],
        help="Type of the solidity compiler binary to be used.",
    )
    run_command.add_argument(
        "--solc-binary-path",
        dest="solc_binary_path",
        type=Path,
        required=True,
        help="Path to the solidity compiler binary.",
    )

    running_mode = run_command.add_mutually_exclusive_group()
    running_mode.add_argument(
        "--run",
        metavar="TEST_NAME",
        dest="selected_tests",
        nargs="+",
        default=[],
        help="List of one or more external tests to run (separated by sapce).",
    )
    running_mode.add_argument(
        "--run-all",
        dest="run_all",
        default=False,
        action="store_true",
        help="Run all available external tests.",
    )

    return parser.parse_args()


def main():
    try:
        args = parse_commandline()
        args.cmd(vars(args))
        return os.EX_OK
    except ExternalTestNotFound as exception:
        print(f"Error: {exception}", file=sys.stderr)
        return os.EX_NOINPUT
    except RuntimeError as exception:
        print(f"Error: {exception}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
