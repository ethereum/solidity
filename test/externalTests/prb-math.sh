#!/usr/bin/env bash

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
# (c) 2022 solidity contributors.
#------------------------------------------------------------------------------

set -e

source scripts/common.sh
source test/externalTests/common.sh

REPO_ROOT=$(realpath "$(dirname "$0")/../..")

verify_input "$@"
BINARY_TYPE="$1"
BINARY_PATH="$(realpath "$2")"
SELECTED_PRESETS="$3"

function compile_fn { yarn compile; }
# NOTE: `yarn test` runs `mocha` which seems to disable the gas reporter.
function test_fn { npx --no hardhat --no-compile test; }

function prb_math_test
{
    local repo="https://github.com/paulrberg/prb-math"
    local ref_type=branch
    local ref=main
    local config_file="hardhat.config.ts"
    local config_var="config"

    local compile_only_presets=()
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize           # Compilation fails with "YulException: Variable var_y_1960 is 8 slot(s) too deep inside the stack."
        #ir-optimize-evm-only     # Compilation fails with "YulException: Variable var_y_1960 is 8 slot(s) too deep inside the stack."
        ir-optimize-evm+yul
        legacy-optimize-evm-only
        legacy-optimize-evm+yul
        legacy-no-optimize
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"

    cp .env.example .env

    # The project has yarn 3.1.0 binary stored in the repo and yarnrc forces the yarn 1.x binary
    # installed system-wide to use it. Unfortunately Yarn 3 fails in weird ways when we remove
    # yarn.lock. Remove the config to restore Yarn 1.x.
    rm .yarnrc.yml

    # Disable tests that won't pass on the ir presets due to Hardhat heuristics. Note that this also disables
    # them for other presets but that's fine - we want same code run for benchmarks to be comparable.
    # TODO: Remove this when Hardhat adjusts heuristics for IR (https://github.com/nomiclabs/hardhat/issues/2115).
    pushd test/contracts/prbMathUd60x18/pure/
    sed -i 's|context(\("when the sum overflows"\)|context.skip(\1|g' add.test.ts
    sed -i 's|context(\("when the sum does not overflow"\)|context.skip(\1|g' add.test.ts
    sed -i 's|context(\("when both operands are zero"\)|context.skip(\1|g' avg.test.ts
    sed -i 's|context(\("when one operand is zero and the other is not zero"\)|context.skip(\1|g' avg.test.ts
    sed -i 's|context(\("when the denominator is zero"\)|context.skip(\1|g' div.test.ts
    sed -i 's|context(\("when x is zero"\)|context.skip(\1|g' inv.test.ts
    popd
    pushd test/contracts/prbMathSd59x18/pure/
    sed -i 's|context(\("when the sum overflows"\)|context.skip(\1|g' add.test.ts
    sed -i 's|context(\("when the sum underflows"\)|context.skip(\1|g' add.test.ts
    sed -i 's|context(\("when the denominator is zero"\)|context.skip(\1|g' div.test.ts
    sed -i 's|context(\("when x is zero"\)|context.skip(\1|g' inv.test.ts
    sed -i 's|context(\("when the difference underflows"\)|context.skip(\1|g' sub.test.ts
    sed -i 's|context(\("when the difference overflows"\)|context.skip(\1|g' sub.test.ts
    popd

    neutralize_package_lock
    neutralize_package_json_hooks
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")" "$config_var"
    yarn install --no-lock-file
    yarn add hardhat-gas-reporter

    replace_version_pragmas

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn "$config_var"
        store_benchmark_report hardhat prb-math "$repo" "$preset"
    done
}

external_test PRBMath prb_math_test
