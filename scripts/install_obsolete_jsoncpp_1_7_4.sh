#!/usr/bin/env sh
set -e

TEMPDIR=$(mktemp -d)
JSONCPP="jsoncpp-1.7.4"
wget -O "$TEMPDIR/$JSONCPP.tar.gz" https://github.com/open-source-parsers/jsoncpp/archive/1.7.4.tar.gz

tar xvzf "$TEMPDIR/${JSONCPP}.tar.gz" -C $TEMPDIR

cd $TEMPDIR/$JSONCPP
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ../..
make
make install

rm -rf $TEMPDIR

