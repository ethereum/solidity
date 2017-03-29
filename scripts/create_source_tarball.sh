#!/usr/bin/env sh
#

set -e

REPO_ROOT="$(dirname "$0")"/..
(
    cd "$REPO_ROOT"
    version=$(grep -oP "PROJECT_VERSION \"?\K[0-9.]+(?=\")"? CMakeLists.txt)
    commithash=$(git rev-parse --short=8 HEAD)
    commitdate=$(git show --format=%ci HEAD | head -n 1 | cut - -b1-10 | sed -e 's/-0?/./' | sed -e 's/-0?/./')

    # file exists and has zero size -> not a prerelease
    if [ -e prerelease.txt -a ! -s prerelease.txt ]
    then
        versionstring="$version"
    else
        versionstring="$version-nightly-$commitdate-$commithash"
    fi

    TEMPDIR=$(mktemp -d)
    SOLDIR="$TEMPDIR/solidity_$versionstring/"
    mkdir "$SOLDIR"
    # Store the current source
    git checkout-index -a --prefix="$SOLDIR"
    git submodule foreach 'git checkout-index -a --prefix="'"$SOLDIR"'/$path/"'
    # Store the commit hash
    echo "$commithash" > "$SOLDIR/commit_hash.txt"
    # Add dependencies
    mkdir -p "$SOLDIR/deps/downloads/" 2>/dev/null || true
    wget -O "$SOLDIR/deps/downloads/jsoncpp-1.7.7.tar.gz" https://github.com/open-source-parsers/jsoncpp/archive/1.7.7.tar.gz
    mkdir -p "$REPO_ROOT/upload"
    tar czf "$REPO_ROOT/upload/solidity_$versionstring.tar.gz" -C "$TEMPDIR" "solidity_$versionstring"
    rm -r "$TEMPDIR"
)
