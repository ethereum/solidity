"""
Import the Solidity test files in Coq
"""
import os
import subprocess
from pqdm.processes import pqdm

test_folders = [
  "../test/libsolidity/semanticTests",
  "../test/libsolidity/syntaxTests",
]

files_to_translate = []

for test_folder in test_folders:
    # Iterate recursively over all the `.sol` files
    for root, _, files in os.walk(test_folder):
        for file in files:
            if file.endswith(".sol"):
                files_to_translate.append((root, file))


def translate_file(root_file):
    root, file = root_file
    # Run `solc`
    result = subprocess.run(
        [
            "../build/solc/solc",
            "-o",
            os.path.join("test", root, file[:-4]),
            "--overwrite",
            "--ir-coq",
            os.path.join(root, file)
        ],
        capture_output=True,
        text=False, # Output in binary format, as sometimes it is not valid Unicode
    )

    # For now we comment this out, as there are too many errors
    # if result.returncode != 0:
    #     print(f"Error in {root}/{file}")
    #     print(result.stderr.decode())
    #     continue

try:
    pqdm(files_to_translate, translate_file, n_jobs=(os.cpu_count() or 1))
except KeyboardInterrupt:
    print("\nProgram stopped with Ctrl-C.")
