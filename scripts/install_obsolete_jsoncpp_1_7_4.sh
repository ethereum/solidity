#!/usr/bin/env bash
set -eu

TEMPDIR=$(mktemp -d)
(
    cd "$TEMPDIR"
    jsoncpp_version="1.7.4"
    jsoncpp_package="jsoncpp-${jsoncpp_version}.tar.gz"
    wget -O "$jsoncpp_package" https://github.com/open-source-parsers/jsoncpp/archive/${jsoncpp_version}.tar.gz
    tar xvzf "$jsoncpp_package"
    cd "jsoncpp-${jsoncpp_version}"
    mkdir -p build
    cd build
    cmake -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ..
    make
    make install
)
rm -r "$TEMPDIR"
