#!/usr/bin/env sh
#

set -e

REPO_ROOT="$(dirname "$0")"/..
(
    cd "$REPO_ROOT"
    version=$(scripts/get_version.sh)
    commithash=$(git rev-parse --short=8 HEAD)
    commitdate=$(git show --format=%ci HEAD | head -n 1 | cut - -b1-10 | sed -e 's/-0?/./' | sed -e 's/-0?/./')

    # file exists and has zero size -> not a prerelease
    if [ -e prerelease.txt ] && [ ! -s prerelease.txt ]
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
    # Store the commit hash
    echo "$commithash" > "$SOLDIR/commit_hash.txt"
    if [ -e prerelease.txt ] && [ ! -s prerelease.txt ]
    then
        cp prerelease.txt "$SOLDIR/"
    fi
    # Add dependencies
    mkdir -p "$SOLDIR/deps/downloads/" 2>/dev/null || true
    jsoncpp_version="1.9.3"
    jsoncpp_package_path="$SOLDIR/deps/downloads/jsoncpp-${jsoncpp_version}.tar.gz"
    jsoncpp_sha256=8593c1d69e703563d94d8c12244e2e18893eeb9a8a9f8aa3d09a327aa45c8f7d
    wget -O "$jsoncpp_package_path" "https://github.com/open-source-parsers/jsoncpp/archive/${jsoncpp_version}.tar.gz"
    if ! [ "$(sha256sum "$jsoncpp_package_path")" = "${jsoncpp_sha256}  ${jsoncpp_package_path}" ]
    then
        >&2 echo "ERROR: Downloaded jsoncpp source package has wrong checksum."
        exit 1
    fi
    mkdir -p "$REPO_ROOT/upload"
    tar --owner 0 --group 0 -czf "$REPO_ROOT/upload/solidity_$versionstring.tar.gz" -C "$TEMPDIR" "solidity_$versionstring"
    rm -r "$TEMPDIR"
)
