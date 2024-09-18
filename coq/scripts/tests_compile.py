"""
Compile the Coq files in the CoqOfSolidityTests directory in parallel.

We cannot use a standard Makefile as `coqdep` is too slow on this directory (several minutes).
"""
import os
from pathlib import Path
import subprocess
from concurrent.futures import ProcessPoolExecutor, as_completed
from multiprocessing import cpu_count
import argparse

# Files that we avoid to compile due to errors
black_list_file = 'scripts/tests_blacklist.txt'
if os.path.exists(black_list_file):
    with open(black_list_file, 'r') as f:
        black_list = f.read().splitlines()


def compile_coq_file(coq_file: Path):
    """Compile a single Coq file usings coqc if it's outdated."""
    vo_file = coq_file.with_suffix('.vo')
    coq_file_mtime = coq_file.stat().st_mtime

    # Check if the .vo file exists and is up-to-date
    if vo_file.exists():
        vo_file_mtime = vo_file.stat().st_mtime
        if vo_file_mtime >= coq_file_mtime:
            return

    # Check if the file is in the black list
    if str(coq_file) in black_list:
        return

    try:
        command = [
            'coqc',
            '-R', 'CoqOfSolidity', 'CoqOfSolidity',
            '-R', 'CoqOfSolidityTests', 'CoqOfSolidityTests',
            '-impredicative-set',
            coq_file
        ]
        print(*command)
        subprocess.run(
            command,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
    except subprocess.CalledProcessError as e:
        print(f"Error compiling {coq_file}:\n{e.stderr.decode()}")


def compile_directory(root_path: Path):
    """Compile all .v files in a directory."""
    coq_files = [f for f in os.listdir(root_path) if f.endswith('.v')]
    has_generated_test = 'GeneratedTest.v' in coq_files
    other_coq_files = [
        root_path / f
        for f in coq_files
        if f != 'GeneratedTest.v'
    ]
    # Compile other .v files first
    for coq_file in sorted(other_coq_files):
        compile_coq_file(coq_file)
    # Then compile GeneratedTest.v
    if has_generated_test:
        generated_test_path = root_path / 'GeneratedTest.v'
        compile_coq_file(generated_test_path)


def main():
    parser = argparse.ArgumentParser(description='Compile Coq files in parallel.')
    parser.add_argument(
        '-j', '--jobs', type=int, default=cpu_count(),
        help='Number of parallel jobs (default: number of CPUs)'
    )
    args = parser.parse_args()

    test_dir = 'CoqOfSolidityTests/'
    # Collect all directories containing .v files
    dirs_to_compile = []
    for root, _dirs, files in os.walk(test_dir):
        if any(f.endswith('.v') for f in files):
            dirs_to_compile.append(Path(root))

    with ProcessPoolExecutor(max_workers=args.jobs) as executor:
        futures = {
            executor.submit(compile_directory, dir_path): dir_path
            for dir_path in dirs_to_compile
        }
        for future in as_completed(futures):
            dir_path = futures[future]
            try:
                future.result()
            except Exception as exc:
                print(f'Exception occurred while compiling directory {dir_path}: {exc}')


if __name__ == '__main__':
    main()
