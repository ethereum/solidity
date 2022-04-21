#! /usr/bin/env python3

"""
Runs mypy on all Python files in project directories known to contain Python scripts.
"""

from argparse import ArgumentParser
from os import path, walk
from textwrap import dedent
import subprocess
import sys

from typing import Any

PROJECT_ROOT = path.dirname(path.dirname(path.realpath(__file__)))
MYPY_CONFIG = f"{PROJECT_ROOT}/scripts/mypy.ini"

SGR_INFO = "\033[1;32m"
SGR_CLEAR = "\033[0m"

def mypy_all_filenames(dev_mode: Any, rootdirs: Any) -> bool:
    """ Performs type annotation checks on all python files within given root directory (recursively).  """

    BARE_COMMAND = [
        "mypy",
        f"--config-file={MYPY_CONFIG}",
        "--namespace-packages",
        "--explicit-package-bases",
        "--install-types",
        "--non-interactive"
    ]

    filenames = []
    for rootdir in rootdirs:
        for rootpath, _, filenames_w in walk(rootdir):
            for filename in filenames_w:
                if filename.endswith('.py'):
                    filenames.append(path.join(rootpath, filename))

    if not dev_mode:
        command_line = BARE_COMMAND + filenames
        return subprocess.run(command_line, check=False).returncode == 0

    for i, filename in enumerate(filenames):
        command_line = BARE_COMMAND + [filename]
        print(
            f"{SGR_INFO}"
            f"[{i + 1}/{len(filenames)}] "
            f"Running mypy on file: {filename}{SGR_CLEAR}"
        )

        process = subprocess.run(command_line, check=False)

        if process.returncode != 0:
            return False

    print()
    return True


def parse_command_line() -> Any:
    script_description = dedent("""
        Runs type annotation checks on all Python files in project directories known to contain Python scripts.

        This script is meant to be run from the CI but can also be easily used in the local dev
        environment.
    """)

    parser = ArgumentParser(description=script_description)
    parser.add_argument(
        '-d', '--dev-mode',
        dest='dev_mode',
        default=False,
        action='store_true',
        help=(
            "Abort on first error. "
            "In this mode every script is passed to mypy separately. "
        )
    )
    return parser.parse_args()


def main() -> None:
    options = parse_command_line()

    rootdirs = [
        f"{PROJECT_ROOT}/docs",
        f"{PROJECT_ROOT}/scripts",
        f"{PROJECT_ROOT}/test",
    ]
    success = mypy_all_filenames(options.dev_mode, rootdirs)

    if not success:
        sys.exit(1)
    else:
        print("No problems found.")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        sys.exit("Interrupted by user. Exiting.")
