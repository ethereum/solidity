#! /usr/bin/env python3
import random
import re
import os
import getopt
import sys
from os import path

ENCODING = "utf-8"
SOURCE_FILE_PATTERN = r"\b\d+_error\b"


def read_file(file_name):
    content = None
    _, tail = path.split(file_name)
    is_latin = tail == "invalid_utf8_sequence.sol"
    try:
        with open(file_name, "r", encoding="latin-1" if is_latin else ENCODING) as f:
            content = f.read()
    finally:
        if content is None:
            print(f"Error reading: {file_name}")
    return content


def write_file(file_name, content):
    with open(file_name, "w", encoding=ENCODING) as f:
        f.write(content)


def in_comment(source, pos):
    slash_slash_pos = source.rfind("//", 0, pos)
    lf_pos = source.rfind("\n", 0, pos)
    if slash_slash_pos > lf_pos:
        return True
    slash_star_pos = source.rfind("/*", 0, pos)
    star_slash_pos = source.rfind("*/", 0, pos)
    return slash_star_pos > star_slash_pos


def find_ids_in_source_file(file_name, id_to_file_names):
    source = read_file(file_name)
    for m in re.finditer(SOURCE_FILE_PATTERN, source):
        if in_comment(source, m.start()):
            continue
        underscore_pos = m.group(0).index("_")
        error_id = m.group(0)[0:underscore_pos]
        if error_id in id_to_file_names:
            id_to_file_names[error_id].append(file_name)
        else:
            id_to_file_names[error_id] = [file_name]


def find_ids_in_source_files(file_names):
    """Returns a dictionary with list of source files for every appearance of every id"""

    id_to_file_names = {}
    for file_name in file_names:
        find_ids_in_source_file(file_name, id_to_file_names)
    return id_to_file_names


def get_next_id(available_ids):
    assert len(available_ids) > 0, "Out of IDs"
    next_id = random.choice(list(available_ids))
    available_ids.remove(next_id)
    return next_id


def fix_ids_in_source_file(file_name, id_to_count, available_ids):
    source = read_file(file_name)

    k = 0
    destination = []
    for m in re.finditer(SOURCE_FILE_PATTERN, source):
        destination.extend(source[k:m.start()])

        underscore_pos = m.group(0).index("_")
        error_id = m.group(0)[0:underscore_pos]

        # incorrect id or id has a duplicate somewhere
        if not in_comment(source, m.start()) and (len(error_id) != 4 or error_id[0] == "0" or id_to_count[error_id] > 1):
            assert error_id in id_to_count
            new_id = get_next_id(available_ids)
            assert new_id not in id_to_count
            id_to_count[error_id] -= 1
        else:
            new_id = error_id

        destination.extend(new_id + "_error")
        k = m.end()

    destination.extend(source[k:])

    destination = ''.join(destination)
    if source != destination:
        write_file(file_name, destination)
        print(f"Fixed file: {file_name}")


def fix_ids_in_source_files(file_names, id_to_count):
    """
    Fixes ids in given source files;
    id_to_count contains number of appearances of every id in sources
    """

    available_ids = {str(error_id) for error_id in range(1000, 10000)} - id_to_count.keys()
    for file_name in file_names:
        fix_ids_in_source_file(file_name, id_to_count, available_ids)


def find_files(top_dir, sub_dirs, extensions):
    """Builds a list of files with given extensions in specified subdirectories"""

    source_file_names = []
    for directory in sub_dirs:
        for root, _, file_names in os.walk(os.path.join(top_dir, directory), onerror=lambda e: sys.exit(f"Walk error: {e}")):
            for file_name in file_names:
                _, ext = path.splitext(file_name)
                if ext in extensions:
                    source_file_names.append(path.join(root, file_name))

    return source_file_names


def find_ids_in_test_file(file_name):
    source = read_file(file_name)
    pattern = r"^// (.*Error|Warning|Info) \d\d\d\d:"
    return {m.group(0)[-5:-1] for m in re.finditer(pattern, source, flags=re.MULTILINE)}


def find_ids_in_test_files(file_names):
    """Returns a set containing all ids in tests"""

    ids = set()
    for file_name in file_names:
        ids |= find_ids_in_test_file(file_name)
    return ids


def find_ids_in_cmdline_test_err(file_name):
    source = read_file(file_name)
    pattern = r' \(\d\d\d\d\):'
    return {m.group(0)[-6:-2] for m in re.finditer(pattern, source, flags=re.MULTILINE)}


def print_ids(ids):
    for k, error_id in enumerate(sorted(ids)):
        if k % 10 > 0:
            print(" ", end="")
        elif k > 0:
            print()
        print(error_id, end="")


def print_ids_per_file(ids, id_to_file_names, top_dir):
    file_name_to_ids = {}
    for error_id in ids:
        for file_name in id_to_file_names[error_id]:
            relpath = path.relpath(file_name, top_dir)
            if relpath not in file_name_to_ids:
                file_name_to_ids[relpath] = []
            file_name_to_ids[relpath].append(error_id)

    for file_name in sorted(file_name_to_ids):
        print(file_name)
        for error_id in sorted(file_name_to_ids[file_name]):
            print(f" {error_id}", end="")
        print()


def examine_id_coverage(top_dir, source_id_to_file_names, new_ids_only=False):
    test_sub_dirs = [
        path.join("test", "libsolidity", "natspecJSON"),
        path.join("test", "libsolidity", "smtCheckerTests"),
        path.join("test", "libsolidity", "syntaxTests"),
        path.join("test", "libyul", "yulSyntaxTests")
    ]
    test_file_names = find_files(
        top_dir,
        test_sub_dirs,
        [".sol", ".yul"]
    )
    source_ids = source_id_to_file_names.keys()
    test_ids = find_ids_in_test_files(test_file_names)

    # special case, we are interested in warnings which are ignored by regular tests:
    # Warning (1878): SPDX license identifier not provided in source file. ....
    # Warning (3420): Source file does not specify required compiler version!
    test_ids |= find_ids_in_cmdline_test_err(path.join(top_dir, "test", "cmdlineTests", "error_codes", "err"))

    # white list of ids which are not covered by tests
    white_ids = {
        "9804", # Tested in test/libyul/ObjectParser.cpp.
        "1544",
        "1749",
        "2674",
        "6367",
        "8387",
        "3805", # "This is a pre-release compiler version, please do not use it in production."
                # The warning may or may not exist in a compiler build.
        "4591", # "There are more than 256 warnings. Ignoring the rest."
                # Due to 3805, the warning lists look different for different compiler builds.
        "1834", # Unimplemented feature error, as we do not test it anymore via cmdLineTests
        "5430", # basefee being used in inline assembly for EVMVersion < london
        "1180", # SMTChecker, covered by CL tests
        "2339", # SMTChecker, covered by CL tests
        "2961", # SMTChecker, covered by CL tests
        "6240", # SMTChecker, covered by CL tests
        "9576", # SMTChecker, covered by CL tests
    }
    assert len(test_ids & white_ids) == 0, "The sets are not supposed to intersect"
    test_ids |= white_ids

    test_only_ids = test_ids - source_ids
    source_only_ids = source_ids - test_ids

    if not new_ids_only:
        print(f"IDs in source files: {len(source_ids)}")
        print(f"IDs in test files  : {len(test_ids)} ({len(test_ids) - len(source_ids)})")
        print()

        if len(test_only_ids) != 0:
            print("Error. The following error codes found in tests, but not in sources:")
            print_ids(test_only_ids)
            return False

        if len(source_only_ids) != 0:
            print("The following error codes found in sources, but not in tests:")
            print_ids_per_file(source_only_ids, source_id_to_file_names, top_dir)
            print("\n\nPlease make sure to add appropriate tests.")
            return False

    old_source_only_ids = {
        "1218", "1584", "1823",
        "1988", "2066", "2833", "3356",
        "3893", "3996", "4010", "4458", "4802",
        "4902", "5272", "5622", "5798", "5840", "7128", "7400",
        "7589", "7593", "7649", "7710",
        "8065", "8084", "8140", "8158",
        "8312", "8592", "9134", "9609",
    }

    new_source_only_ids = source_only_ids - old_source_only_ids
    if len(new_source_only_ids) != 0:
        print("The following new error code(s), not covered by tests, found:")
        print_ids(new_source_only_ids)
        print(
            "\nYou can:\n"
            "- create appropriate test(s);\n"
            "- add the error code(s) to old_source_only_ids in error_codes.py\n"
            "  (to silence the checking script, with a promise to add a test later);\n"
            "- add the error code(s) to white_ids in error_codes.py\n"
            "  (for rare cases when the error is not supposed to be tested)"
        )
        return False

    return True


def main(argv):
    check = False
    fix = False
    no_confirm = False
    examine_coverage = False
    next_id = False
    opts, _args = getopt.getopt(argv, "", ["check", "fix", "no-confirm", "examine-coverage", "next"])

    for opt, _arg in opts:
        if opt == "--check":
            check = True
        elif opt == "--fix":
            fix = True
        elif opt == "--no-confirm":
            no_confirm = True
        elif opt == "--examine-coverage":
            examine_coverage = True
        elif opt == "--next":
            next_id = True

    if [check, fix, examine_coverage, next_id].count(True) != 1:
        print("usage: python error_codes.py --check | --fix [--no-confirm] | --examine-coverage | --next")
        sys.exit(1)

    cwd = os.getcwd()

    source_file_names = find_files(
        cwd,
        ["libevmasm", "liblangutil", "libsolc", "libsolidity", "libsolutil", "libyul", "solc"],
        [".h", ".cpp"]
    )
    source_id_to_file_names = find_ids_in_source_files(source_file_names)

    ok = True
    for error_id in sorted(source_id_to_file_names):
        if len(error_id) != 4:
            print(f"ID {error_id} length != 4")
            ok = False
        if error_id[0] == "0":
            print(f"ID {error_id} starts with zero")
            ok = False
        if len(source_id_to_file_names[error_id]) > 1:
            print(f"ID {error_id} appears {len(source_id_to_file_names[error_id])} times")
            ok = False

    if examine_coverage:
        if not ok:
            print("Incorrect IDs have to be fixed before applying --examine-coverage")
            sys.exit(1)
        res = 0 if examine_id_coverage(cwd, source_id_to_file_names) else 1
        sys.exit(res)

    ok &= examine_id_coverage(cwd, source_id_to_file_names, new_ids_only=True)

    random.seed()

    if next_id:
        if not ok:
            print("Incorrect IDs have to be fixed before applying --next")
            sys.exit(1)
        available_ids = {str(error_id) for error_id in range(1000, 10000)} - source_id_to_file_names.keys()
        next_id = get_next_id(available_ids)
        print(f"Next ID: {next_id}")
        sys.exit(0)

    if ok:
        print("No incorrect IDs found")
        sys.exit(0)

    if check:
        sys.exit(1)

    assert fix, "Unexpected state, should not come here without --fix"

    if not no_confirm:
        answer = input(
            "\nDo you want to fix incorrect IDs?\n"
            "Please commit current changes first, and review the results when the script finishes.\n"
            "[Y/N]? "
        )
        while len(answer) == 0 or answer not in "YNyn":
            answer = input("[Y/N]? ")
        if answer not in "yY":
            sys.exit(1)

    # number of appearances for every id
    source_id_to_count = { error_id: len(file_names) for error_id, file_names in source_id_to_file_names.items() }

    fix_ids_in_source_files(source_file_names, source_id_to_count)
    print("Fixing completed")
    sys.exit(2)


if __name__ == "__main__":
    main(sys.argv[1:])
