#! /usr/bin/env python3

"""
Performs pylint on all python files in the project repo's {test,script,docs} directory recursively.

This script is meant to be run from the CI but can also be easily in local dev environment,
where you can optionally pass `-d` as command line argument to let this script abort on first error.
"""

from os import path, walk, system
from sys import argv, exit as exitwith

PROJECT_ROOT = path.dirname(path.realpath(__file__))
PYLINT_RCFILE = "{}/pylintrc".format(PROJECT_ROOT)

SGR_INFO = "\033[1;32m"
SGR_CLEAR = "\033[0m"

def pylint_all_filenames(dev_mode, rootdirs):
    """ Performs pylint on all python files within given root directory (recursively).  """
    filenames = []
    for rootdir in rootdirs:
        for rootpath, _, filenames_w in walk(rootdir):
            for filename in filenames_w:
                if filename.endswith('.py'):
                    filenames.append(path.join(rootpath, filename))

    checked_count = 0
    failed = []
    for filename in filenames:
        checked_count += 1
        cmdline = "pylint --rcfile=\"{}\" \"{}\"".format(PYLINT_RCFILE, filename)
        print("{}[{}/{}] Running pylint on file: {}{}".format(SGR_INFO, checked_count, len(filenames),
                                                              filename, SGR_CLEAR))
        exit_code = system(cmdline)
        if exit_code != 0:
            if dev_mode:
                return 1, checked_count
            failed.append(filename)

    return len(failed), len(filenames)

def main():
    """ Collects all python script root dirs and runs pylint on them. You can optionally
        pass `-d` as command line argument to let this script abort on first error. """
    dev_mode = len(argv) == 2 and argv[1] == "-d"
    failed_count, total_count = pylint_all_filenames(dev_mode, [
        path.abspath(path.dirname(__file__) + "/../docs"),
        path.abspath(path.dirname(__file__) + "/../scripts"),
        path.abspath(path.dirname(__file__) + "/../test")])
    if failed_count != 0:
        exitwith("pylint failed on {}/{} files.".format(failed_count, total_count))
    else:
        print("Successfully tested {} files.".format(total_count))

if __name__ == "__main__":
    main()
