#!/usr/bin/env bash
set -eu

TEMPDIR=$(mktemp -d)
(
    cd "$TEMPDIR"
    jsoncpp_version="1.9.5"
    jsoncpp_package="jsoncpp-${jsoncpp_version}.tar.gz"
    jsoncpp_sha256=f409856e5920c18d0c2fb85276e24ee607d2a09b5e7d5f0a371368903c275da2
    wget -O "$jsoncpp_package" https://github.com/open-source-parsers/jsoncpp/archive/${jsoncpp_version}.tar.gz
    if ! [ "$(sha256sum "$jsoncpp_package")" = "${jsoncpp_sha256}  ${jsoncpp_package}" ]
    then
        >&2 echo "ERROR: Downloaded jsoncpp source package has wrong checksum."
        exit 1
    fi
    tar xvzf "$jsoncpp_package"
    cd "jsoncpp-${jsoncpp_version}"
    mkdir -p build
    cd build
    cmake -DCMAKE_OSX_ARCHITECTURES:STRING="x86_64;arm64" -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ..
    make
    make install
)
rm -r "$TEMPDIR"
