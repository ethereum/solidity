#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script for installing pre-requisite packages for cpp-ethereum.
#
# The documentation for cpp-ethereum is hosted at:
#
# http://www.ethdocs.org/en/latest/ethereum-clients/cpp-ethereum/
#
# (c) 2016 cpp-ethereum contributors.
#------------------------------------------------------------------------------

if [[ "$OSTYPE" == "darwin"* ]]; then

    if echo `sw_vers` | grep "10.11"; then
        echo OS X El Capitan detected
    elif echo `sw_vers` | grep "10.10"; then
        echo OS X Yosemite detected
    else
        echo Unsupported OS X version.  We only support Yosemite and El Capitan.
        exit 1
    fi

    brew update
    brew upgrade

    brew install boost
    brew install cmake
    brew install cryptopp
    brew install miniupnpc
    brew install leveldb
    brew install gmp
    brew install jsoncpp
    brew install libmicrohttpd
    brew install libjson-rpc-cpp
    brew install homebrew/versions/llvm37

elif [[ "$OSTYPE" == "linux-gnu" ]]; then

    # NOTE - These steps are ONLY correct for Ubuntu Trusty.  We need to add
    # further conditionals in here for the other Ubuntu versions, and carry on
    # from there if we then want to get this pattern working for Debian,
    # OpenSUSE, Fedora, Arch Linux, Raspian, FreeBSD, etc.

    # Add additional PPAs which we need to be able to build cpp-ethereum on
    # Ubuntu Trusty.  That includes our own PPAs and a PPA for getting CMake 3.x
    # on Trusty.
    sudo add-apt-repository -y ppa:ethereum/ethereum
    sudo add-apt-repository -y ppa:ethereum/ethereum-dev
    sudo apt-add-repository -y ppa:george-edison55/cmake-3.x
    sudo apt-get -y update

    # Install binaries for nearly all of our dependencies
    sudo apt-get -y install \
        python-sphinx \
        build-essential \
        cmake \
        git \
        libboost-all-dev \
        libcurl4-openssl-dev \
        libcryptopp-dev \
        libgmp-dev \
        libjsoncpp-dev \
        libleveldb-dev \
        libmicrohttpd-dev \
        libminiupnpc-dev \
        libz-dev \
        opencl-headers

    # The exception is libjson-rpc-cpp, which we have to build from source for
    # reliable results.   The only binaries available for this package are those
    # we made ourselves against the (now very old) v0.4.2 release, which are unreliable,
    # so instead we build the latest release label (v0.6.0) from source, which works just
    # fine.   We should update our PPA.
    #
    # See https://github.com/ethereum/webthree-umbrella/issues/513
    #
    # Hmm.   Arachnid is still getting this issue on OS X, which already has v0.6.0, so
    # it isn't as simple as just updating all our builds to that version, though that is
    # sufficient for us to get CircleCI and TravisCI working.   We still haven't got to
    # the bottom of this issue, and are going to need to debug it in some scenario where
    # we can reproduce it 100%, which MIGHT end up being within our automation here, but
    # against a build-from-source-with-extra-printfs() of v0.4.2.
    sudo apt-get -y install libargtable2-dev libedit-dev
    git clone git://github.com/cinemast/libjson-rpc-cpp.git
    cd libjson-rpc-cpp
    git checkout v0.6.0
    mkdir build
    cd build
    cmake .. -DCOMPILE_TESTS=NO
    make
    sudo make install
    sudo ldconfig
    cd ../..

    # And install the English language package and reconfigure the locales.
    # We really shouldn't need to do this, and should instead force our locales to "C"
    # within our application runtimes, because this issue shows up on multiple Linux distros,
    # and each will need fixing in the install steps, where we should really just fix it once
    # in the code.
    #
    # See https://github.com/ethereum/webthree-umbrella/issues/169
    sudo apt-get -y install language-pack-en-base
    sudo dpkg-reconfigure locales

fi
