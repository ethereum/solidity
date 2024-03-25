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
# (c) 2024 solidity contributors.
# ------------------------------------------------------------------------------

import os
import sys
import subprocess
from pathlib import Path
from shutil import which

# Our scripts/ is not a proper Python package so we need to modify PYTHONPATH to import from it
# pragma pylint: disable=import-error,wrong-import-position
PROJECT_ROOT = Path(__file__).parents[2]
sys.path.insert(0, f"{PROJECT_ROOT}/scripts/externalTests")

from runners.base import run_test
from runners.base import TestConfig
from runners.foundry import FoundryRunner
from test_helpers import SettingsPreset

class ChainlinkRunner(FoundryRunner):
    def configure(self):
        if which("pnpm") is None:
            raise RuntimeError("pnpm not found.")

        os.chdir(self.test_dir)
        # pnpm install seems to fail to install sha3 package
        # but the tests don't need it
        subprocess.run(
            ["pnpm", "install", "sha3"],
            env=self.env,
            check=True
        )

        subprocess.run(
            ["pnpm", "install"],
            env=self.env,
            check=True
        )

        super().configure()

test_config = TestConfig(
    name="Chainlink",
    repo_url="https://github.com/smartcontractkit/chainlink.git",
    ref="develop",
    test_dir=Path("contracts"),
    compile_only_presets=[
    ],
    settings_presets=[
        SettingsPreset.LEGACY_NO_OPTIMIZE,
        SettingsPreset.LEGACY_OPTIMIZE_EVM_ONLY,
        SettingsPreset.LEGACY_OPTIMIZE_EVM_YUL,
        # pylint: disable=line-too-long
        SettingsPreset.IR_NO_OPTIMIZE,       # Error: Yul exception:Variable expr_15699_address is 2 slot(s) too deep inside the stack. Stack too deep.
        SettingsPreset.IR_OPTIMIZE_EVM_ONLY, # Error: Yul exception:Variable expr_15699_address is 2 slot(s) too deep inside the stack. Stack too deep.
        SettingsPreset.IR_OPTIMIZE_EVM_YUL,
    ],
)

sys.exit(run_test(ChainlinkRunner(argv=sys.argv[1:], config=test_config)))
