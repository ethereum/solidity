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
    try:
        with open(file_name, "r", encoding=ENCODING) as f:
            content = f.read()
    finally:
        if content == None:
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


def find_ids_in_source_file(file_name, ids):
    source = read_file(file_name)
    for m in re.finditer(SOURCE_FILE_PATTERN, source):
        if in_comment(source, m.start()):
            continue
        underscore_pos = m.group(0).index("_")
        id = m.group(0)[0:underscore_pos]
        if id in ids:
            ids[id] += 1
        else:
            ids[id] = 1


def get_used_ids(file_names):
    used_ids = {}
    for file_name in file_names:
        find_ids_in_source_file(file_name, used_ids)
    return used_ids


def get_next_id(available_ids):
    assert len(available_ids) > 0, "Out of IDs"
    next_id = random.choice(list(available_ids))
    available_ids.remove(next_id)
    return next_id


def fix_ids_in_file(file_name, available_ids, used_ids):
    source = read_file(file_name)

    k = 0
    destination = []
    for m in re.finditer(SOURCE_FILE_PATTERN, source):
        destination.extend(source[k:m.start()])

        underscore_pos = m.group(0).index("_")
        id = m.group(0)[0:underscore_pos]

        # incorrect id or id has a duplicate somewhere
        if not in_comment(source, m.start()) and (len(id) != 4 or id[0] == "0" or used_ids[id] > 1):
            assert id in used_ids
            new_id = get_next_id(available_ids)
            assert new_id not in used_ids
            used_ids[id] -= 1
        else:
            new_id = id

        destination.extend(new_id + "_error")
        k = m.end()

    destination.extend(source[k:])

    destination = ''.join(destination)
    if source != destination:
        write_file(file_name, destination)
        print(f"Fixed file: {file_name}")


def fix_ids(used_ids, file_names):
    available_ids = {str(id) for id in range(1000, 10000)} - used_ids.keys()
    for file_name in file_names:
        fix_ids_in_file(file_name, available_ids, used_ids)


def find_files(top_dir, sub_dirs, extensions):
    """Builds a list of files with given extensions in specified subdirectories"""

    source_file_names = []
    for dir in sub_dirs:
        for root, _, file_names in os.walk(os.path.join(top_dir, dir), onerror=lambda e: exit(f"Walk error: {e}")):
            for file_name in file_names:
                _, ext = path.splitext(file_name)
                if ext in extensions:
                    source_file_names.append(path.join(root, file_name))

    return source_file_names


def find_ids_in_test_file(file_name):
    source = read_file(file_name)
    pattern = r"^// (.*Error|Warning) \d\d\d\d:"
    return {m.group(0)[-5:-1] for m in re.finditer(pattern, source, flags=re.MULTILINE)}


def find_ids_in_test_files(file_names):
    used_ids = set()
    for file_name in file_names:
        used_ids |= find_ids_in_test_file(file_name)
    return used_ids


def find_ids_in_cmdline_test_err(file_name):
    source = read_file(file_name)
    pattern = r' \(\d\d\d\d\):'
    return {m.group(0)[-6:-2] for m in re.finditer(pattern, source, flags=re.MULTILINE)}


def print_ids(ids):
    for k, id in enumerate(sorted(ids)):
        if k % 10 > 0:
            print(" ", end="")
        elif k > 0:
            print()
        print(id, end="")


def examine_id_coverage(top_dir, used_ids):
    test_sub_dirs = [
        path.join("test", "libsolidity", "errorRecoveryTests"),
        path.join("test", "libsolidity", "smtCheckerTests"),
        path.join("test", "libsolidity", "syntaxTests")
    ]
    test_file_names = find_files(
        top_dir,
        test_sub_dirs,
        [".sol"]
    )
    covered_ids = find_ids_in_test_files(test_file_names)

    # special case, we are interested in warnings which are ignored by regular tests:
    # Warning (1878): SPDX license identifier not provided in source file. ....
    # Warning (3420): Source file does not specify required compiler version!
    covered_ids |= find_ids_in_cmdline_test_err(path.join(top_dir, "test", "cmdlineTests", "error_codes", "err"))

    print(f"IDs in source files: {len(used_ids)}")
    print(f"IDs in test files  : {len(covered_ids)} ({len(covered_ids) - len(used_ids)})")
    print()

    unused_covered_ids = covered_ids - used_ids
    if len(unused_covered_ids) != 0:
        print("Error. The following error codes found in tests, but not in sources:")
        print_ids(unused_covered_ids)
        return 1

    used_uncovered_ids = used_ids - covered_ids
    if len(used_uncovered_ids) != 0:
        print("The following error codes found in sources, but not in tests:")
        print_ids(used_uncovered_ids)
        print("\n\nPlease make sure to add appropriate tests.")
        return 1

    return 0


def main(argv):
    # pylint: disable=too-many-branches, too-many-locals, too-many-statements

    check = False
    fix = False
    no_confirm = False
    examine_coverage = False
    next = False
    opts, args = getopt.getopt(argv, "", ["check", "fix", "no-confirm", "examine-coverage", "next"])

    for opt, arg in opts:
        if opt == '--check':
            check = True
        elif opt == "--fix":
            fix = True
        elif opt == '--no-confirm':
            no_confirm = True
        elif opt == '--examine-coverage':
            examine_coverage = True
        elif opt == '--next':
            next = True

    if [check, fix, examine_coverage, next].count(True) != 1:
        print("usage: python error_codes.py --check | --fix [--no-confirm] | --examine-coverage | --next")
        exit(1)

    cwd = os.getcwd()

    source_file_names = find_files(
        cwd,
        ["libevmasm", "liblangutil", "libsolc", "libsolidity", "libsolutil", "libyul", "solc"],
        [".h", ".cpp"]
    )
    used_ids = get_used_ids(source_file_names)

    ok = True
    for id in sorted(used_ids):
        if len(id) != 4:
            print(f"ID {id} length != 4")
            ok = False
        if id[0] == "0":
            print(f"ID {id} starts with zero")
            ok = False
        if used_ids[id] > 1:
            print(f"ID {id} appears {used_ids[id]} times")
            ok = False

    if examine_coverage:
        if not ok:
            print("Incorrect IDs have to be fixed before applying --examine-coverage")
        res = examine_id_coverage(cwd, used_ids.keys())
        exit(res)

    random.seed()

    if next:
        if not ok:
            print("Incorrect IDs have to be fixed before applying --next")
        available_ids = {str(id) for id in range(1000, 10000)} - used_ids.keys()
        next_id = get_next_id(available_ids)
        print(f"Next ID: {next_id}")
        exit(0)

    if ok:
        print("No incorrect IDs found")
        exit(0)

    if check:
        exit(1)

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
            exit(1)

    fix_ids(used_ids, source_file_names)
    print("Fixing completed")
    exit(2)


if __name__ == "__main__":
    main(sys.argv[1:])
