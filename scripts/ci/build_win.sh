#!/usr/bin/env bash
set -euo pipefail

(( $# <= 1 )) || { >&2 echo "Usage: $0 [PRERELEASE_SOURCE]"; exit 1; }
prerelease_source="${1:-ci}"
ROOTDIR="$(realpath "$(dirname "$0")/../../")"
FORCE_RELEASE="${FORCE_RELEASE:-}"
CIRCLE_TAG="${CIRCLE_TAG:-}"
CMAKE_VS_GLOBALS_ARG=()

cd "$ROOTDIR"
"${ROOTDIR}/scripts/prerelease_suffix.sh" "$prerelease_source" "$CIRCLE_TAG" > prerelease.txt

export CCACHE_DIR="$HOME/.ccache"
export CCACHE_BASEDIR="$ROOTDIR"
export CCACHE_NOHASHDIR=1
export CCACHE_COMPILERTYPE=msvc
VSWHERE="${VSWHERE:-C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe}"
VS_INSTALL="$("$VSWHERE" -latest -products "*" \
  -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
  -property installationPath | tr -d '\r')"
VCTOOLS_VERSION="$(tr -d '\r' < "$VS_INSTALL/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt")"
CCACHE_COMPILER="$VS_INSTALL/VC/Tools/MSVC/$VCTOOLS_VERSION/bin/Hostx64/x64/cl.exe"
CCACHE_COMPILER="${CCACHE_COMPILER//\\//}"
export CCACHE_COMPILER
PATH="$ROOTDIR/deps/ccache:$(dirname "$CCACHE_COMPILER"):$PATH"
export PATH
# Use a Windows-style path for Visual Studio globals.
WIN_PWD="$(pwd -W)"
# Visual Studio generator doesn't use compiler launchers; use ccache masquerade.
# See https://github.com/ccache/ccache/wiki/MS-Visual-Studio
CMAKE_VS_GLOBALS="CLToolPath=${WIN_PWD}/deps/ccache;UseMultiToolTask=true"
CMAKE_VS_GLOBALS_ARG=(-DCMAKE_VS_GLOBALS="$CMAKE_VS_GLOBALS")
mkdir -p "$CCACHE_DIR"

mkdir -p build/
cd build/

# NOTE: Using an array to force Bash to do wildcard expansion
boost_dir=("${ROOTDIR}/deps/boost/lib/cmake/Boost-"*)

 # shellcheck disable=SC2086
 "${ROOTDIR}/deps/cmake/bin/cmake" \
    -G "Visual Studio 17 2022" \
    -DBoost_DIR="${boost_dir[*]}" \
    -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded \
    -DCMAKE_INSTALL_PREFIX="${ROOTDIR}/uploads/" \
    "${CMAKE_VS_GLOBALS_ARG[@]}" \
    ${CMAKE_OPTIONS:-} \
    ..
ccache -z
MSBuild.exe solidity.sln \
    -property:Configuration=Release \
    -maxCpuCount:10 \
    -verbosity:minimal
"${ROOTDIR}/deps/cmake/bin/cmake" \
    --build . \
    -j 10 \
    --target install \
    --config Release
ccache -s
