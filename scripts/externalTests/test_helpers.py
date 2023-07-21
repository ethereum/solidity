#!/usr/bin/env python3

# ------------------------------------------------------------------------------
# This file is part of solidity.
#
# solidity is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# solidity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with solidity.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2023 solidity contributors.
# ------------------------------------------------------------------------------

import os
import re
import subprocess
import sys
from argparse import ArgumentParser
from enum import Enum
from pathlib import Path
from typing import List
from typing import Set

# Our scripts/ is not a proper Python package so we need to modify PYTHONPATH to import from it
# pragma pylint: disable=import-error,wrong-import-position
PROJECT_ROOT = Path(__file__).parents[2]
sys.path.insert(0, f"{PROJECT_ROOT}/scripts/common")

from git_helpers import git_commit_hash

SOLC_FULL_VERSION_REGEX = re.compile(r"^[a-zA-Z: ]*(.*)$")
SOLC_SHORT_VERSION_REGEX = re.compile(r"^([0-9.]+).*\+|\-$")


class SettingsPreset(Enum):
    LEGACY_NO_OPTIMIZE = 'legacy-no-optimize'
    IR_NO_OPTIMIZE = 'ir-no-optimize'
    LEGACY_OPTIMIZE_EVM_ONLY = 'legacy-optimize-evm-only'
    IR_OPTIMIZE_EVM_ONLY = 'ir-optimize-evm-only'
    LEGACY_OPTIMIZE_EVM_YUL = 'legacy-optimize-evm+yul'
    IR_OPTIMIZE_EVM_YUL = 'ir-optimize-evm+yul'


def compiler_settings(evm_version: str, via_ir: bool = False, optimizer: bool = False, yul: bool = False) -> dict:
    return {
        "optimizer": {"enabled": optimizer, "details": {"yul": yul}},
        "evmVersion": evm_version,
        "viaIR": via_ir,
    }


def settings_from_preset(preset: SettingsPreset, evm_version: str) -> dict:
    return {
        SettingsPreset.LEGACY_NO_OPTIMIZE:       compiler_settings(evm_version),
        SettingsPreset.IR_NO_OPTIMIZE:           compiler_settings(evm_version, via_ir=True),
        SettingsPreset.LEGACY_OPTIMIZE_EVM_ONLY: compiler_settings(evm_version, optimizer=True),
        SettingsPreset.IR_OPTIMIZE_EVM_ONLY:     compiler_settings(evm_version, via_ir=True, optimizer=True),
        SettingsPreset.LEGACY_OPTIMIZE_EVM_YUL:  compiler_settings(evm_version, optimizer=True, yul=True),
        SettingsPreset.IR_OPTIMIZE_EVM_YUL:      compiler_settings(evm_version, via_ir=True, optimizer=True, yul=True),
    }[preset]


def parse_custom_presets(presets: List[str]) -> Set[SettingsPreset]:
    return {SettingsPreset(p) for p in presets}

def parse_command_line(description: str, args: List[str]):
    arg_parser = ArgumentParser(description)
    arg_parser.add_argument(
        "solc_binary_type",
        metavar="solc-binary-type",
        type=str,
        default="native",
        choices=["native", "solcjs"],
        help="""Solidity compiler binary type""",
    )
    arg_parser.add_argument(
        "solc_binary_path",
        metavar="solc-binary-path",
        type=Path,
        help="""Path to solc binary""",
    )
    arg_parser.add_argument(
        "selected_presets",
        metavar="selected-presets",
        help="""List of compiler settings presets""",
        nargs='*',
    )
    return arg_parser.parse_args(args)


def download_project(test_dir: Path, repo_url: str, ref_type: str = "branch", ref: str = "master"):
    assert ref_type in ("commit", "branch", "tag")

    print(f"Cloning {ref_type} {ref} of {repo_url}...")
    if ref_type == "commit":
        os.mkdir(test_dir)
        os.chdir(test_dir)
        subprocess.run(["git", "init"], check=True)
        subprocess.run(["git", "remote", "add", "origin", repo_url], check=True)
        subprocess.run(["git", "fetch", "--depth", "1", "origin", ref], check=True)
        subprocess.run(["git", "reset", "--hard", "FETCH_HEAD"], check=True)
    else:
        os.chdir(test_dir.parent)
        subprocess.run(["git", "clone", "--no-progress", "--depth", "1", repo_url, "-b", ref, test_dir.resolve()], check=True)
        if not test_dir.exists():
            raise RuntimeError("Failed to clone the project.")
        os.chdir(test_dir)

    if (test_dir / ".gitmodules").exists():
        subprocess.run(["git", "submodule", "update", "--init"], check=True)

    print(f"Current commit hash: {git_commit_hash()}")


def parse_solc_version(solc_version_string: str) -> str:
    solc_version_match = re.search(SOLC_FULL_VERSION_REGEX, solc_version_string)
    if solc_version_match is None:
        raise RuntimeError(f"Solc version could not be found in: {solc_version_string}.")
    return solc_version_match.group(1)


def get_solc_short_version(solc_full_version: str) -> str:
    solc_short_version_match = re.search(SOLC_SHORT_VERSION_REGEX, solc_full_version)
    if solc_short_version_match is None:
        raise RuntimeError(f"Error extracting short version string from: {solc_full_version}.")
    return solc_short_version_match.group(1)


def store_benchmark_report(self):
    raise NotImplementedError()


def replace_version_pragmas(test_dir: Path):
    """
    Replace fixed-version pragmas (part of Consensys best practice).
    Include all directories to also cover node dependencies.
    """
    print("Replacing fixed-version pragmas...")
    for source in test_dir.glob("**/*.sol"):
        content = source.read_text(encoding="utf-8")
        content = re.sub(r"pragma solidity [^;]+;", r"pragma solidity >=0.0;", content)
        with open(source, "w", encoding="utf-8") as f:
            f.write(content)
