#! /bin/bash
#------------------------------------------------------------------------------
# Bash script to execute the Solidity tests by CircleCI.
#
# The documentation for solidity is hosted at:
#
#     https://docs.soliditylang.org
#
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
# (c) 2016-2019 solidity contributors.
# ------------------------------------------------------------------------------
set -e

REPODIR="$(realpath $(dirname $0)/..)"

EVM_VALUES=(homestead byzantium constantinople petersburg istanbul)
OPTIMIZE_VALUES=(0 1)
STEPS=$(( 2 + ${#EVM_VALUES[@]} * ${#OPTIMIZE_VALUES[@]} ))

if (( $CIRCLE_NODE_TOTAL )) && (( $CIRCLE_NODE_TOTAL > 1 ))
then
    # Run step 1 as the only step on the first executor
    # and evenly distribute the other steps among
    # the other executors.
    # The first step takes much longer than the other steps.
    if (( $CIRCLE_NODE_INDEX == 0 ))
    then
        RUN_STEPS="1"
    else
        export CIRCLE_NODE_INDEX=$(($CIRCLE_NODE_INDEX - 1))
        export CIRCLE_NODE_TOTAL=$(($CIRCLE_NODE_TOTAL - 1))
        RUN_STEPS=$(seq 2 "$STEPS" | circleci tests split)
    fi
else
    RUN_STEPS=$(seq "$STEPS")
fi

# turn newlines into spaces
RUN_STEPS=$(echo $RUN_STEPS)

echo "Running steps $RUN_STEPS..."

STEP=1

# Run SMTChecker tests separately, as the heaviest expected run.
[[ " $RUN_STEPS " =~ " $STEP " ]] && EVM=istanbul OPTIMIZE=1 ABI_ENCODER_V1=1 BOOST_TEST_ARGS="-t smtCheckerTests/*" "${REPODIR}/.circleci/soltest.sh"
STEP=$(($STEP + 1))

# Run without SMTChecker tests.
[[ " $RUN_STEPS " =~ " $STEP " ]] && EVM=istanbul OPTIMIZE=1 ABI_ENCODER_V1=1 BOOST_TEST_ARGS="-t !smtCheckerTests" "${REPODIR}/.circleci/soltest.sh"
STEP=$(($STEP + 1))

for OPTIMIZE in ${OPTIMIZE_VALUES[@]}
do
    for EVM in ${EVM_VALUES[@]}
    do
        # run tests against hera ewasm evmc vm, only if OPTIMIZE == 0 and evm version is byzantium
        EWASM_ARGS=""
        [ "${EVM}" = "byzantium" ] && [ "${OPTIMIZE}" = "0" ] && EWASM_ARGS="--ewasm"

        [[ " $RUN_STEPS " =~ " $STEP " ]] && EVM="$EVM" OPTIMIZE="$OPTIMIZE" SOLTEST_FLAGS="$SOLTEST_FLAGS $EWASM_ARGS" BOOST_TEST_ARGS="-t !@nooptions" "${REPODIR}/.circleci/soltest.sh"
        STEP=$(($STEP + 1))
    done
done

if (($STEP != $STEPS + 1))
then
    echo "Step counter not properly adjusted!" >2
    exit 1
fi
