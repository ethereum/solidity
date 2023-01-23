#!/usr/bin/env bash
set -eu

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

TEMPDIR=$(mktemp -d)
(
    cd "$TEMPDIR"
    jsoncpp_version="1.7.4"
    jsoncpp_package="jsoncpp-${jsoncpp_version}.tar.gz"
    jsoncpp_sha256=10dcd0677e80727e572a1e462193e51a5fde3e023b99e144b2ee1a469835f769
    wget -O "$jsoncpp_package" https://github.com/open-source-parsers/jsoncpp/archive/${jsoncpp_version}.tar.gz
    validate_checksum "$jsoncpp_package" "$jsoncpp_sha256"
    tar xvzf "$jsoncpp_package"
    cd "jsoncpp-${jsoncpp_version}"
    mkdir -p build
    cd build
    cmake -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ..
    make
    make install
)
rm -r "$TEMPDIR"
