#!/usr/bin/env bash

set -e

READLINK=readlink
if [[ "$OSTYPE" == "darwin"* ]]; then
    READLINK=greadlink
fi
ROOT_DIR=$(${READLINK} -f "$(dirname "$0")"/..)
WORKDIR="${ROOT_DIR}/build/antlr"
ANTLR_JAR="${ROOT_DIR}/build/deps/antlr4.jar"
ANTLR_JAR_URI="https://www.antlr.org/download/antlr-4.8-complete.jar"
BATCH_SIZE=100  # Process 100 files per JVM invocation

SGR_RESET="\033[0m"
SGR_BOLD="\033[1m"
SGR_GREEN="\033[32m"
SGR_RED="\033[31m"
SGR_BLUE="\033[34m"

function download_antlr4
{
  if [[ ! -e "$ANTLR_JAR" ]]
  then
    curl -o "${ANTLR_JAR}" "${ANTLR_JAR_URI}"
  fi
}

function prepare_workdir
{
  mkdir -p "${ROOT_DIR}/build/deps"
  mkdir -p "${WORKDIR}"
  mkdir -p "${WORKDIR}/src"
  mkdir -p "${WORKDIR}/target"
}

prepare_workdir
download_antlr4

echo "Creating parser"
(
cd "${ROOT_DIR}"/docs/grammar
# Create lexer/parser from grammar
java -jar "${ANTLR_JAR}" SolidityParser.g4 SolidityLexer.g4 -o "${WORKDIR}/src/"

# Compile lexer/parser sources
javac -classpath "${ANTLR_JAR}" "${WORKDIR}/src/"*.java -d "${WORKDIR}/target/"

# Compile AntlrBatchTestRig
javac -classpath "${ANTLR_JAR}:${WORKDIR}/target/" "${ROOT_DIR}/scripts/AntlrBatchTestRig.java" -d "${WORKDIR}/target/"
)

# we only want to use files that do not contain excluded parser errors, analysis errors or multi-source files.
SOL_FILES=()
while IFS='' read -r line
do
  SOL_FILES+=("$line")
done < <(
  grep --include "*.sol" -riL -E \
    "^\/\/ (Syntax|Type|Declaration)Error|^\/\/ ParserError (1684|2837|3716|3997|5333|6275|6281|6933|7319)|^==== Source:" \
    "${ROOT_DIR}/test/libsolidity/syntaxTests" \
    "${ROOT_DIR}/test/libsolidity/semanticTests" |
      # Skipping the unicode tests as I couldn't adapt the lexical grammar to recursively counting RLO/LRO/PDF's.
      grep -v -E 'comments/.*_direction_override.*.sol' |
      grep -v -E 'comments/.*_direction_isolate.*.sol' |
      grep -v -E 'literals/.*_direction_override.*.sol' |
      # Skipping a test with "revert E;" because ANTLR cannot distinguish it from
      # a variable declaration.
      grep -v -E 'revertStatement/non_called.sol' |
      # Skipping tests with "let basefee := ..."
      grep -v -E 'inlineAssembly/basefee_berlin_function.sol' |
      grep -v -E 'inlineAssembly/basefee_pre_london.sol' |
      # Skipping a test with "let blobbasefee := ..."
      grep -v -E 'inlineAssembly/blobbasefee_shanghai_function.sol' |
      # Skipping a test with "let mcopy := ..."
      grep -v -E 'inlineAssembly/mcopy_as_identifier_pre_cancun.sol' |
      # Skipping tests with "let prevrandao := ..."
      grep -v -E 'inlineAssembly/prevrandao_allowed_function_pre_paris.sol' |
      grep -v -E 'inlineAssembly/prevrandao_disallowed_function_post_paris.sol' |
      # Skipping a test with "let blobhash := ..."
      grep -v -E 'inlineAssembly/blobhash_pre_cancun.sol' |
      grep -v -E 'inlineAssembly/blobhash_pre_cancun_not_reserved.sol' |
      # Skipping a test with "let clz := ..."
      grep -v -E 'inlineAssembly/clz_pre_osaka.sol' |
      # Skipping a test with "let slotnum := ..."
      grep -v -E 'inlineAssembly/slotnum_pre_amsterdam_function.sol' |
      # Skipping tests with "let tstore/tload := ..."
      grep -v -E 'inlineAssembly/tload_tstore_not_reserved_before_cancun.sol' |
      # Skipping license error, unrelated to the grammar
      grep -v -E 'license/license_double5.sol' |
      grep -v -E 'license/license_hidden_unicode.sol' |
      grep -v -E 'license/license_unicode.sol' |
      # Skipping tests with 'something.address' as 'address' as the grammar fails on those
      grep -v -E 'inlineAssembly/external_function_pointer_address.*.sol'
)

YUL_FILES=()
# Add all yul optimizer tests without objects and types.
while IFS='' read -r line
do
  YUL_FILES+=("$line")
done < <(
  grep -riL -E \
    "object|\:[ ]*[uib]" \
    "${ROOT_DIR}/test/libyul/yulOptimizerTests"
)

# Combine all files into one array
ALL_FILES=("${SOL_FILES[@]}" "${YUL_FILES[@]}")
num_tests=${#ALL_FILES[@]}
failed_count=0
test_count=0

echo "Testing $num_tests files..."

# Process all files in batches
for ((i=0; i<${#ALL_FILES[@]}; i+=BATCH_SIZE)); do
  batch=("${ALL_FILES[@]:i:BATCH_SIZE}")
  batch_size=${#batch[@]}

  echo "Processing batch $((i/BATCH_SIZE + 1)) (${batch_size} files)..."

  # Run batch
  output=$(java -classpath "${ANTLR_JAR}:${WORKDIR}/target/" AntlrBatchTestRig "${batch[@]}" 2>&1) || true

  # Parse output
  while IFS= read -r line; do
    if [[ -z "$line" ]]; then
      continue
    fi

    test_count=$((test_count + 1))

    if [[ "$line" =~ ^PASS:(.+):(.*) ]]; then
      file="${BASH_REMATCH[1]}"
      status="${BASH_REMATCH[2]}"
      if [[ "$status" == "FAILED_AS_EXPECTED" ]]; then
        echo -e "${SGR_BLUE}[${test_count}/${num_tests}] ${file}${SGR_RESET} ${SGR_BOLD}${SGR_GREEN}FAILED AS EXPECTED${SGR_RESET}"
      else
        echo -e "${SGR_BLUE}[${test_count}/${num_tests}] ${file}${SGR_RESET} ${SGR_BOLD}${SGR_GREEN}OK${SGR_RESET}"
      fi
    elif [[ "$line" =~ ^FAIL:(.+):(.*) ]]; then
      file="${BASH_REMATCH[1]}"
      error="${BASH_REMATCH[2]}"
      echo -e "${SGR_BLUE}[${test_count}/${num_tests}] ${file}${SGR_RESET} ${SGR_BOLD}${SGR_RED}FAILED${SGR_RESET}"
      echo "$error"
      failed_count=$((failed_count + 1))
    elif [[ "$line" =~ ^ERROR:(.+):(.*) ]]; then
      file="${BASH_REMATCH[1]}"
      error="${BASH_REMATCH[2]}"
      echo -e "${SGR_BLUE}[${test_count}/${num_tests}] ${file}${SGR_RESET} ${SGR_BOLD}${SGR_RED}ERROR${SGR_RESET}"
      echo "$error"
      failed_count=$((failed_count + 1))
    fi
  done <<< "$output"
done

echo "Summary: ${failed_count} of $num_tests sources failed."
exit ${failed_count}
