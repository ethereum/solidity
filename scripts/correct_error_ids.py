import random
import re
import os
from os import path

ENCODING = "utf-8"
PATTERN = r"\b\d+_error\b"


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


def find_ids_in_file(file_name, ids):
    source = read_file(file_name)
    for m in re.finditer(PATTERN, source):
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
        find_ids_in_file(file_name, used_ids)
    return used_ids


def get_id(available_ids, used_ids):
    while len(available_ids) > 0:
        random.seed(len(available_ids))
        k = random.randrange(len(available_ids))
        id = list(available_ids.keys())[k]
        del available_ids[id]
        if id not in used_ids:
            return id
    assert False, "Out of IDs"


def fix_ids_in_file(file_name, available_ids, used_ids):
    source = read_file(file_name)

    k = 0
    destination = []
    for m in re.finditer(PATTERN, source):
        destination.extend(source[k:m.start()])

        underscore_pos = m.group(0).index("_")
        id = m.group(0)[0:underscore_pos]

        # incorrect id or id has a duplicate somewhere
        if not in_comment(source, m.start()) and (len(id) != 4 or id[0] == "0" or used_ids[id] > 1):
            assert id in used_ids
            new_id = get_id(available_ids, used_ids)
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
    available_ids = {str(id): None for id in range(1000, 10000)}
    for file_name in file_names:
        fix_ids_in_file(file_name, available_ids, used_ids)


def find_source_files(top_dir):
    """Builds the list of .h and .cpp files in top_dir directory"""

    source_file_names = []
    dirs = ['libevmasm', 'liblangutil', 'libsolc', 'libsolidity', 'libsolutil', 'libyul', 'solc']

    for dir in dirs:
        for root, _, file_names in os.walk(os.path.join(top_dir, dir), onerror=lambda e: exit(f"Walk error: {e}")):
            for file_name in file_names:
                _, ext = path.splitext(file_name)
                if ext in [".h", ".cpp"]:
                    source_file_names.append(path.join(root, file_name))

    return source_file_names


def main():
    cwd = os.getcwd()
    answer = input(
        f"This script checks and corrects *_error literals in .h and .cpp files\n"
        f"in {cwd}, recursively.\n\n"
        f"Please commit current changes first, and review the results when the script finishes.\n\n"
        f"Do you want to start [Y/N]? "
    )
    while len(answer) == 0 or answer not in "YNyn":
        answer = input("[Y/N]? ")
    if answer not in "yY":
        return

    source_file_names = find_source_files(cwd)

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

    if ok:
        print("No incorrect IDs found")
    else:
        fix_ids(used_ids, source_file_names)
        print("Fixing compteted")


if __name__ == "__main__":
    main()
