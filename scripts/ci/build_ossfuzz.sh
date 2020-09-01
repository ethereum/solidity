#!/usr/bin/env bash
set -ex

ROOTDIR="/root/project"
BUILDDIR="${ROOTDIR}/build"
mkdir -p "${BUILDDIR}" && mkdir -p "$BUILDDIR/deps"

ANTLRJAR="${ROOTDIR}/build/deps/antlr4.8.jar"
ANTLRJAR_URI="https://www.antlr.org/download/antlr-4.8-complete.jar"

download_antlr4()
{
  if [[ ! -e "${ANTLRJAR}" ]]
  then
    wget -O "${ANTLRJAR}" "${ANTLRJAR_URI}"
  fi
}

generate_protobuf_bindings()
{
  cd "${ROOTDIR}"/test/tools/ossfuzz
  # Generate protobuf C++ bindings
  for protoName in yul abiV2 sol;
  do
    protoc "${protoName}"Proto.proto --cpp_out .
  done
}

generate_antlr4_bindings()
{
  cd "${ROOTDIR}"
  # Replace boolean with bool to suit c++ syntax
  sed -i 's/boolean /bool /g' docs/grammar/Solidity.g4
  # Generate antlr4 visitor/parser/lexer c++ bindings
  java -jar "${ANTLRJAR}" -Dlanguage=Cpp \
    -Xexact-output-dir -package solidity::test::fuzzer -o test/tools/ossfuzz \
    -no-listener -visitor docs/grammar/SolidityLexer.g4 docs/grammar/Solidity.g4
  # Delete unnecessary autogen files
  rm -f "${ROOTDIR}"/test/tools/ossfuzz/Solidity*Visitor.cpp \
    "${ROOTDIR}"/test/tools/ossfuzz/Solidity*.interp \
    "${ROOTDIR}"/test/tools/ossfuzz/Solidity*.tokens
}

build_fuzzers()
{
  cd "${BUILDDIR}"
  cmake .. -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}" \
    -DCMAKE_TOOLCHAIN_FILE="${ROOTDIR}"/cmake/toolchains/libfuzzer.cmake
  make ossfuzz ossfuzz_proto ossfuzz_abiv2 -j 4
}

download_antlr4
generate_protobuf_bindings
generate_antlr4_bindings
build_fuzzers