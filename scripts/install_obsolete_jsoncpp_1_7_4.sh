#!/usr/bin/env sh
set -e

TEMPDIR=$(mktemp -d)
(
    cd $TEMPDIR
    wget https://github.com/open-source-parsers/jsoncpp/archive/1.7.4.tar.gz
    tar xvzf "1.7.4.tar.gz"
    cd "jsoncpp-1.7.4"
    mkdir -p build
    cd build
    cmake -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ..
    make
    make install
)
rm -rf $TEMPDIR
