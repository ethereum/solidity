"""
Import the Solidity test files in Coq
"""
import os
from pathlib import Path
from pqdm.processes import pqdm
import subprocess

test_root = Path("../test/libsolidity")

test_folders = [
    Path("semanticTests"),
    Path("syntaxTests"),
]

files_to_translate: list[Path] = []

for test_folder in test_folders:
    # Iterate recursively over all the `.sol` files
    for root, _, files in os.walk(test_root / test_folder):
        for file in files:
            if file.endswith(".sol"):
                files_to_translate.append(Path(root) / Path(file))


def translate_file(path: Path):
    # Run `solc`
    result = subprocess.run(
        [
            "../build/solc/solc",
            "-o",
            Path("CoqOfSolidityTests") / path.relative_to(test_root).with_suffix(""),
            "--overwrite",
            "--optimize",
            "--ir-coq",
            path
        ],
        capture_output=True,
        text=False, # Output in binary format, as sometimes it is not valid Unicode
    )

    # For now we comment this out, as there are too many errors
    # if result.returncode != 0:
    #     print(f"Error in {root}/{file}")
    #     print(result.stderr.decode())

try:
    pqdm(files_to_translate, translate_file, n_jobs=(os.cpu_count() or 1))
except KeyboardInterrupt:
    print("\nProgram stopped with Ctrl-C.")
