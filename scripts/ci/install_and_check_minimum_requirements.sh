#!/usr/bin/env bash
set -euo pipefail

# These versions must be kept in sync with the docs in `docs/installing-solidity.rst#building-from-source`.

# minimum boost version
BOOST_VERSION=1.83

# minimum cmake version
CMAKE_MAJOR=3
CMAKE_MINOR=21
CMAKE_PATCH=3
CMAKE_FULL_VERSION="${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH}"
CMAKE_FILENAME="cmake-${CMAKE_FULL_VERSION}-linux-x86_64"
CMAKE_TARBALL="${CMAKE_FILENAME}.tar.gz"
# Cache dir for cmake set in config.yml
CMAKE_DOWNLOAD_DIR="${CMAKE_DOWNLOAD_DIR:-${HOME}/.cache/cmake-download}"

# minimum gcc/clang versions
GCC_VERSION=13.3.0
CLANG_VERSION=19.1.1

# which compiler version to check in this script
check_gcc=false
check_clang=false

while (( $# > 0 )); do
    case "$1" in
        --gcc)
            check_gcc=true
            shift
            ;;
        --clang)
            check_clang=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--gcc] [--clang]"
            exit 1
            ;;
    esac
done

echo "-- Removing boost and CMake from the system"
sudo apt-get --quiet=2 remove --purge 'libboost*'
sudo apt-get --quiet=2 remove --purge cmake

echo "-- Installing Boost ${BOOST_VERSION}"
sudo apt-get --quiet=2 update
sudo apt-get --quiet=2 install libboost${BOOST_VERSION}-all-dev
installed_boost_version=$(dpkg-query --showformat='${Version}' --show libboost${BOOST_VERSION}-all-dev 2>/dev/null || echo "none")
if [[ $installed_boost_version != ${BOOST_VERSION}* ]]; then
    echo "Error: installed version of boost is $installed_boost_version, expected $BOOST_VERSION"
    exit 1
fi

echo "-- Installing CMake ${CMAKE_FULL_VERSION}"
mkdir --parents "${CMAKE_DOWNLOAD_DIR}"
if [[ -f "${CMAKE_DOWNLOAD_DIR}/${CMAKE_TARBALL}" ]]; then
    echo "-- Using cached ${CMAKE_TARBALL}"
else
    wget --output-document "${CMAKE_DOWNLOAD_DIR}/${CMAKE_TARBALL}" "https://github.com/Kitware/CMake/releases/download/v${CMAKE_FULL_VERSION}/${CMAKE_TARBALL}"
fi
tar --extract --gzip --file "${CMAKE_DOWNLOAD_DIR}/${CMAKE_TARBALL}"
sudo mv "${CMAKE_FILENAME}" "/opt/cmake-${CMAKE_FULL_VERSION}"
sudo ln --symbolic "/opt/cmake-${CMAKE_FULL_VERSION}/bin/"* /usr/local/bin/
echo "-- Installed $(cmake --version)"

if [[ "$check_gcc" == true ]]; then
    installed_gcc_version=$(gcc -dumpfullversion -dumpversion || echo "none")
    if [[ "$installed_gcc_version" != "$GCC_VERSION" ]]; then
        echo "Error: installed version of gcc is $installed_gcc_version, expected $GCC_VERSION"
        exit 1
    fi
    echo "-- gcc version check passed: $installed_gcc_version"
fi

if [[ "$check_clang" == true ]]; then
    installed_clang_version=$(clang -dumpfullversion -dumpversion || echo "none")
    if [[ "$installed_clang_version" != "$CLANG_VERSION" ]]; then
        echo "Error: installed version of clang is $installed_clang_version, expected $CLANG_VERSION"
        exit 1
    fi
    echo "-- clang version check passed: $installed_clang_version"
fi
