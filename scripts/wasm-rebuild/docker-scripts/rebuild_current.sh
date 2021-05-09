#!/bin/bash -e

# Do not call this script directly.

# This script is expected to be run inside the docker image trzeci/emscripten:sdk-tag-1.39.3-64bit and
# be called by ./rebuild_tags.sh.

echo "========== STAGE 1: PREPARE ========== ($(date))"
COMMIT_DATE="$(git show -s --format=%cI HEAD)"
git rev-parse --short=8 HEAD >commit_hash.txt
echo -e "" >prerelease.txt
sed -i -e 's/-Wl,--gc-sections//' cmake/EthCompilerSettings.cmake
echo "set(CMAKE_CXX_FLAGS \"\${CMAKE_CXX_FLAGS} -s EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap','addFunction','removeFunction','UTF8ToString','lengthBytesUTF8','_malloc','stringToUTF8','setValue'] -s WASM=1 -s WASM_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 -Wno-almost-asm\")" >>cmake/EthCompilerSettings.cmake
# Needed for < 0.5.0.
sed -i -e 's/-Werror/-Wno-error/' cmake/EthCompilerSettings.cmake

echo "========== STAGE 2: BUILD ========== ($(date))"
scripts/travis-emscripten/install_deps.sh
if [ -d cryptopp ]; then
  # Needed for < 0.4.4. Will not affect >= 0.4.5.
  # Unfortunately we need to update to the latest
  # release in the 5.6 series for it to build.
  # Hopefully we don't miss any bugs.
  rm -rf cryptopp
  git clone https://github.com/weidai11/cryptopp/
  (
    set -e
    cd cryptopp
    git checkout CRYPTOPP_5_6_5
    ln -s . src
  )
fi
if [ -d jsoncpp ]; then
  # Needed for < 0.4.4. Will not affect >= 0.4.5.
  (
    set -e
    cd jsoncpp
    # Checkout the latest commit at the time of our release.
    git checkout "$(git rev-list -1 --before="$COMMIT_DATE" master)"
  )
fi

set +e

if [ -e scripts/ci/build_emscripten.sh ]; then
  scripts/ci/build_emscripten.sh
else
  # The script used to be in scripts/ci/ in earlier versions.
  scripts/travis-emscripten/build_emscripten.sh
fi

set -e

mkdir -p upload

if [ ! -f upload/soljson.js ]; then
  if [ -f build/solc/soljson.js ]; then
    cp build/solc/soljson.js upload
  elif [ -f build/libsolc/soljson.js ]; then
    cp build/libsolc/soljson.js upload
  elif [ -f emscripten_build/solc/soljson.js ]; then
    cp emscripten_build/solc/soljson.js upload
  elif [ -f emscripten_build/libsolc/soljson.js ]; then
    cp emscripten_build/libsolc/soljson.js upload
  fi
fi

if [ -f upload/soljson.js ]; then
  echo "========== SUCCESS ========== ($(date))"
  exit 0
else
  echo "========== FAILURE ========== ($(date))"
  exit 1
fi
