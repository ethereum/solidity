#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to determine the percentage of tests that are compilable via Yul.
#
# Usage:
#  ./yul_coverage.sh [--no-stats] [--successful] [--internal-compiler-errors]
#                                 [--unimplemented-feature-errors] [--other-errors] [--list-files]
#
#    --no-stats                     will not print the stats to stdout
#    --successful                   print output of successful test-case compilations to stdout
#    --internal-compiler-errors     print output of test-case compilations that resulted in
#                                   internal compilation errors to stdout
#    --unimplemented-feature-errors print output of test-case compilations that resulted in
#                                   unimplemented feature errors to stdout
#    --other-errors                 print output of test-case compilations that resulted in
#                                   errors that where not internal compiler errors or unimplemented feature errors
#                                   to stdout
#    --list-files                   will not print the compiler output to stdout, it will just print the files
#                                   e.g. ./yul_coverage.sh --successful --list-files will just return a list of
#                                   files where it's compilation result was successful
#   Environment Variables
#     SOLC can be set to change the used compiler.
#
#   ./yul_coverage.sh
#   run the script without any parameters to execute the tests will return stats.
#
#   SOLC=<path-to-solc> ./yul_coverage.sh
#   To change the used compiler, just set the SOLC environment variable.
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
# (c) 2020 solidity contributors.
#------------------------------------------------------------------------------

set -e

ROOT_DIR="$(dirname "$0")"/..

for arg in "$@"; do
  case "$arg" in
  --no-stats) NO_STATS=1 ;;
  --successful) SHOW_SUCCESSFUL=1 ;;
  --internal-compiler-errors) SHOW_INTERNAL_COMPILER_ERRORS=1 ;;
  --unimplemented-feature-errors) SHOW_UNIMPLEMENTED_FEATURE_ERRORS=1 ;;
  --other-errors) SHOW_OTHER_ERRORS=1 ;;
  --list-files) ONLY_LIST_FILES=1 ;;
  *)
    echo "Usage:"
    echo "  $(basename "${0}") [--no-stats] [--successful] [--internal-compiler-errors] [--unimplemented-feature-errors] [--other-errors] [--list-files]"
    echo "  --no-stats                     will not print the stats to stdout"
    echo "  --successful                   print output of successful test-case compilations to stdout"
    echo "  --internal-compiler-errors     print output of test-case compilations that resulted in"
    echo "                                 internal compilation errors to stdout"
    echo "  --unimplemented-feature-errors print output of test-case compilations that resulted in"
    echo "                                 unimplemented feature errors to stdout"
    echo "  --other-errors                 print output of test-case compilations that resulted in"
    echo "                                 errors that where not internal compiler errors or unimplemented feature errors"
    echo "                                 to stdout"
    echo "  --list-files                   will not print the compiler output to stdout, it will just print the files"
    echo "                                 e.g. './yul_coverage.sh --successful --list-files' will just return a list of"
    echo "                                 files where it's compilation result was successful"
    exit 0
    ;;
  esac
done

show_output_if() {
  local VAR=${1}
  if [ -n "${VAR}" ]; then
    echo "${SOL_FILE}"
    if [ -z "${ONLY_LIST_FILES}" ]; then
      echo "${OUTPUT}"
      echo ""
    fi
  fi
}

FAILED=()
SUCCESS=()
SOLC=${SOLC:-"$(command -v -- solc)"}
if [ ! -f "${SOLC}" ]; then
  echo "error: solc '${SOLC}' not found."
  exit 1
fi

test_file() {
  local SOL_FILE
  local OUTPUT
  SOL_FILE=${1}

  if OUTPUT=$("${SOLC}" --ir "${SOL_FILE}" 2>&1); then
    SUCCESS+=("${SOL_FILE}")
    show_output_if ${SHOW_SUCCESSFUL}
  else
    FAILED+=("${SOL_FILE}")
    if [[ ${OUTPUT} == *"UnimplementedFeatureError"* ]]; then
      UNIMPLEMENTED_FEATURE_ERRORS+=("${SOL_FILE}")
      show_output_if ${SHOW_UNIMPLEMENTED_FEATURE_ERRORS}
    elif [[ ${OUTPUT} == *"InternalCompilerError"* ]]; then
      INTERNAL_COMPILER_ERRORS+=("${SOL_FILE}")
      show_output_if ${SHOW_INTERNAL_COMPILER_ERRORS}
    else
      OTHER_ERRORS+=("${SOL_FILE}")
      show_output_if ${SHOW_OTHER_ERRORS}
    fi
  fi
}

# we only want to use files that do not contain errors or multi-source files.
SOL_FILES=()
while IFS='' read -r line; do
  SOL_FILES+=("$line")
done < <(
  grep -riL -E \
    "^\/\/ (DocstringParsing|Syntax|Type|Parser|Declaration)Error|^==== Source:" \
    "${ROOT_DIR}/test/libsolidity/syntaxTests" \
    "${ROOT_DIR}/test/libsolidity/semanticTests"
)

for SOL_FILE in "${SOL_FILES[@]}"; do
  test_file "${SOL_FILE}"
done

if [ -z "${NO_STATS}" ]; then
  SUM=$((${#SUCCESS[@]} + ${#FAILED[@]}))
  PERCENTAGE=$(echo "scale=4; ${#SUCCESS[@]} / ${SUM}" | bc)
  echo "${#SUCCESS[@]} / ${SUM} = ${PERCENTAGE}"
  echo "UnimplementedFeatureError(s): ${#UNIMPLEMENTED_FEATURE_ERRORS[@]}"
  echo "InternalCompilerError(s): ${#INTERNAL_COMPILER_ERRORS[@]}"
  echo "OtherError(s): ${#OTHER_ERRORS[@]}"
fi
