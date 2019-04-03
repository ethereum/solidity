#!/usr/bin/env sh
set -e

TEMPDIR="src"
cd /
mkdir -p $TEMPDIR
(
    cd $TEMPDIR
    git clone --depth 1 https://github.com/google/libprotobuf-mutator.git
    mkdir -p LPM
    cd LPM
    cmake ../libprotobuf-mutator -GNinja -DLIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF=ON -DLIB_PROTO_MUTATOR_TESTING=OFF -DCMAKE_BUILD_TYPE=Release && ninja install
)
