#!/usr/bin/env bash
set -e

ROOTDIR="$(dirname "$0")/../.."
BUILDDIR="${ROOTDIR}/build"

mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"

protoc --proto_path=../test/tools/ossfuzz yulProto.proto --cpp_out=../test/tools/ossfuzz
protoc --proto_path=../test/tools/ossfuzz abiV2Proto.proto --cpp_out=../test/tools/ossfuzz
protoc --proto_path=../test/tools/ossfuzz solProto.proto --cpp_out=../test/tools/ossfuzz
cmake .. -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}" -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/libfuzzer.cmake

make ossfuzz ossfuzz_proto ossfuzz_abiv2 -j 4
