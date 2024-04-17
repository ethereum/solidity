#!/usr/bin/env python3

import os
import sys
from pathlib import Path
from textwrap import dedent

# pylint: disable=wrong-import-position
PROJECT_ROOT = Path(__file__).parents[3]
sys.path.insert(0, str(PROJECT_ROOT / "scripts"))

from common.cmdline_helpers import add_preamble
from common.cmdline_helpers import inside_temporary_dir
from common.cmdline_helpers import save_bytecode
from common.cmdline_helpers import solc_bin_report
from common.git_helpers import git_diff
from splitSources import split_sources


@inside_temporary_dir(Path(__file__).parent.name)
def test_bytecode_equivalence():
    source_file_path = Path(__file__).parent / "inputs.sol"
    split_sources(source_file_path, suppress_output=True)
    add_preamble(Path.cwd())

    solc_binary = os.environ.get("SOLC")
    if solc_binary is None:
        raise RuntimeError(
            dedent(
                """\
            `solc` compiler not found.
            Please ensure you set the SOLC environment variable
            with the correct path to the compiler's binary.
        """
            )
        )

    # Repro for https://github.com/ethereum/solidity/issues/14829
    save_bytecode(
        Path("B.bin"),
        solc_bin_report(
            solc_binary,
            [Path("B.sol")],
            via_ir=True,
            optimize=True,
            yul_optimizations="a:",
        ),
        contract="B",
    )
    save_bytecode(
        Path("AB.bin"),
        solc_bin_report(
            solc_binary,
            [Path("A.sol"), Path("B.sol")],
            via_ir=True,
            optimize=True,
            yul_optimizations="a:",
        ),
        contract="B",
    )
    return git_diff(Path("B.bin"), Path("AB.bin"))


if __name__ == "__main__":
    sys.exit(test_bytecode_equivalence())
