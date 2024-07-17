#!/usr/bin/env bash
set -ex

ROOTDIR="$(dirname "$0")/../.."
# shellcheck source=scripts/common.sh
source "${ROOTDIR}/scripts/common.sh"

prerelease_source="${1:-ci}"

cd "${ROOTDIR}"

# shellcheck disable=SC2166
if [ "$CIRCLE_BRANCH" = release -o -n "$CIRCLE_TAG" -o -n "$FORCE_RELEASE" ]
then
    echo -n >prerelease.txt
else
    # Use last commit date rather than build date to avoid ending up with builds for
    # different platforms having different version strings (and therefore producing different bytecode)
    # if the CI is triggered just before midnight.
    TZ=UTC git show --quiet --date="format-local:%Y.%-m.%-d" --format="${prerelease_source}.%cd" >prerelease.txt
fi

if [ -n "$CIRCLE_SHA1" ]
then
    echo -n "$CIRCLE_SHA1" >commit_hash.txt
fi

mkdir -p build
cd build

# shellcheck disable=SC2166
[ -n "$COVERAGE" -a "$CIRCLE_BRANCH" != release -a -z "$CIRCLE_TAG" ] && CMAKE_OPTIONS="$CMAKE_OPTIONS -DCOVERAGE=ON"

# shellcheck disable=SC2086
cmake .. -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}" $CMAKE_OPTIONS -G "Unix Makefiles"

make
