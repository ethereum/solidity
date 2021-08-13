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

SGR_RESET="\033[0m"
SGR_BOLD="\033[1m"
SGR_GREEN="\033[32m"
SGR_RED="\033[31m"
SGR_BLUE="\033[34m"

vt_cursor_up() { echo -ne "\033[A"; }
vt_cursor_begin_of_line() { echo -ne "\r"; }

download_antlr4()
{
  if [[ ! -e "$ANTLR_JAR" ]]
  then
    curl -o "${ANTLR_JAR}" "${ANTLR_JAR_URI}"
  fi
}

prepare_workdir()
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
)

# Run tests
failed_count=0
test_file()
{
  local SOL_FILE
  SOL_FILE="$(${READLINK}  -m "${1}")"
  local cur=${2}
  local max=${3}
  local solOrYul=${4}

  echo -e "${SGR_BLUE}[${cur}/${max}] Testing ${SOL_FILE}${SGR_RESET} ..."
  local output
  if [[ "${solOrYul}" == "sol" ]]; then
    output=$(
      grep -v "^==== ExternalSource:" "${SOL_FILE}" | java \
        -classpath "${ANTLR_JAR}:${WORKDIR}/target/" \
        "org.antlr.v4.gui.TestRig" \
        Solidity \
        sourceUnit 2>&1
    )
  else
    output=$(
      echo "assembly $(cat "${SOL_FILE}")" | java \
        -classpath "${ANTLR_JAR}:${WORKDIR}/target/" \
        "org.antlr.v4.gui.TestRig" \
        Solidity \
        assemblyStatement 2>&1
    )
  fi
  vt_cursor_up
  vt_cursor_begin_of_line
  if grep -qE "^\/\/ ParserError" "${SOL_FILE}"; then
    if [[ "${output}" != "" ]]
    then
      echo -e "${SGR_BLUE}[${cur}/${max}] Testing ${SOL_FILE}${SGR_RESET} ${SGR_BOLD}${SGR_GREEN}FAILED AS EXPECTED${SGR_RESET}"
    else
      echo -e "${SGR_BLUE}[${cur}/${max}] Testing ${SOL_FILE}${SGR_RESET} ${SGR_BOLD}${SGR_RED}SUCCEEDED DESPITE PARSER ERROR${SGR_RESET}"
      echo "${output}"
      failed_count=$((failed_count + 1))
      exit 1
    fi
  else
    if [[ "${output}" == "" ]]
    then
      echo -e "${SGR_BLUE}[${cur}/${max}] Testing ${SOL_FILE}${SGR_RESET} ${SGR_BOLD}${SGR_GREEN}OK${SGR_RESET}"
    else
      echo -e "${SGR_BLUE}[${cur}/${max}] Testing ${SOL_FILE}${SGR_RESET} ${SGR_BOLD}${SGR_RED}FAILED${SGR_RESET}"
      echo "${output}"
      failed_count=$((failed_count + 1))
      exit 1
    fi
  fi
}

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
      grep -v -E 'literals/.*_direction_override.*.sol' |
      # Skipping a test with "revert E;" because ANTLR cannot distinguish it from
      # a variable declaration.
      grep -v -E 'revertStatement/non_called.sol' |
      # Skipping a test with "let basefee := ..."
      grep -v -E 'inlineAssembly/basefee_berlin_function.sol'
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

num_tests=$((${#SOL_FILES[*]} + ${#YUL_FILES[*]}))
test_count=0
for SOL_FILE in "${SOL_FILES[@]}"
do
  test_count=$((test_count + 1))
  test_file "${SOL_FILE}" ${test_count} $num_tests "sol"
done
for YUL_FILE in "${YUL_FILES[@]}"
do
  test_count=$((test_count + 1))
  test_file "${YUL_FILE}" ${test_count} $num_tests "yul"
done

echo "Summary: ${failed_count} of $num_tests sources failed."
exit ${failed_count}
