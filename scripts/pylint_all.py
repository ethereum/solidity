#! /usr/bin/env python3

"""
Runs pylint on all Python files in project directories known to contain Python scripts.
"""

from argparse import ArgumentParser
from os import path, walk
from textwrap import dedent
import subprocess
import sys

PROJECT_ROOT = path.dirname(path.dirname(path.realpath(__file__)))
PYLINT_RCFILE = f"{PROJECT_ROOT}/scripts/pylintrc"

SGR_INFO = "\033[1;32m"
SGR_CLEAR = "\033[0m"

def pylint_all_filenames(dev_mode, rootdirs):
    """ Performs pylint on all python files within given root directory (recursively).  """

    BARE_COMMAND = [
        "pylint",
        f"--rcfile={PYLINT_RCFILE}",
        "--output-format=colorized",
        "--score=n"
    ]

    filenames = []
    for rootdir in rootdirs:
        for rootpath, _, filenames_w in walk(rootdir):
            for filename in filenames_w:
                if filename.endswith('.py'):
                    filenames.append(path.join(rootpath, filename))

    if not dev_mode:
        # NOTE: We could just give pylint the directories and it would find the files on its
        # own but it would treat them as packages, which would result in lots of import errors.
        command_line = BARE_COMMAND + filenames
        return subprocess.run(command_line, check=False).returncode == 0

    for i, filename in enumerate(filenames):
        command_line = BARE_COMMAND + [filename]
        print(
            f"{SGR_INFO}"
            f"[{i + 1}/{len(filenames)}] "
            f"Running pylint on file: {filename}{SGR_CLEAR}"
        )

        process = subprocess.run(command_line, check=False)

        if process.returncode != 0:
            return False

    print()
    return True


def parse_command_line():
    script_description = dedent("""
        Runs pylint on all Python files in project directories known to contain Python scripts.

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
            "In this mode every script is passed to pylint separately. "
        )
    )
    return parser.parse_args()


def main():
    options = parse_command_line()

    rootdirs = [
        f"{PROJECT_ROOT}/docs",
        f"{PROJECT_ROOT}/scripts",
        f"{PROJECT_ROOT}/test",
    ]
    success = pylint_all_filenames(options.dev_mode, rootdirs)

    if not success:
        sys.exit(1)
    else:
        print("No problems found.")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        sys.exit("Interrupted by user. Exiting.")
