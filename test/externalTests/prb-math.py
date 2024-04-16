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

class PRBMathRunner(FoundryRunner):
    def configure(self):
        if which("pnpm") is None:
            raise RuntimeError("pnpm not found.")

        self.setup_presets_profiles()
        # Starting from version v4.0.2, the default installation method for PRBMath dependencies
        # has transitioned from Foundry to Node.js.
        subprocess.run(
            ["pnpm", "install", "--no-frozen-lockfile"],
            env=self.env,
            check=True
        )

test_config = TestConfig(
    name="PRBMath",
    repo_url="https://github.com/PaulRBerg/prb-math.git",
    ref="<latest-release>",
    compile_only_presets=[
        # pylint: disable=line-too-long
        # SettingsPreset.IR_NO_OPTIMIZE,       # Error: Yul exception:Variable expr_15699_address is 2 slot(s) too deep inside the stack. Stack too deep.
        # SettingsPreset.IR_OPTIMIZE_EVM_ONLY, # Error: Yul exception:Variable expr_15699_address is 2 slot(s) too deep inside the stack. Stack too deep.
    ],
    settings_presets=[
        SettingsPreset.LEGACY_NO_OPTIMIZE,
        SettingsPreset.LEGACY_OPTIMIZE_EVM_ONLY,
        SettingsPreset.LEGACY_OPTIMIZE_EVM_YUL,
        SettingsPreset.IR_OPTIMIZE_EVM_YUL,
    ],
)

sys.exit(run_test(PRBMathRunner(argv=sys.argv[1:], config=test_config)))
