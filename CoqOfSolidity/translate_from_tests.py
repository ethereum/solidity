"""
Import the Solidity test files in Coq
"""
import os
import subprocess
from tqdm import tqdm

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

try:
    for index, (root, file) in enumerate(tqdm(files_to_translate)):
        # Run `solc`
        result = subprocess.run(
            [
                "../build/solc/solc",
                "-o",
                os.path.join("test", root),
                "--overwrite",
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
except KeyboardInterrupt:
    print("\nProgram stopped with Ctrl-C.")
