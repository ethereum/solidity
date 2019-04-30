#!/usr/bin/env sh
set -e
TEMPDIR=$(mktemp -d)
(
    cd $TEMPDIR
    svn co https://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/fuzzer libfuzzer
    mkdir -p build-libfuzzer
    cd build-libfuzzer
    CXXFLAGS="-O1 -stdlib=libstdc++"
    $CXX $CXXFLAGS -std=c++11 -O2 -fPIC -c ../libfuzzer/*.cpp -I../libfuzzer
    ar r /usr/lib/libFuzzingEngine.a *.o
)
rm -rf $TEMPDIR
