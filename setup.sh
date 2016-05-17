#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script for installing pre-requisite packages for solidity.
#
# The documentation for solidity is hosted at:
#
# http://solidity.readthedocs.org
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
    brew install jsoncpp

elif [[ "$OSTYPE" == "linux-gnu" ]]; then

    # NOTE - These steps are ONLY correct for Ubuntu Trusty.  We need to add
    # further conditionals in here for the other Ubuntu versions, and carry on
    # from there if we then want to get this pattern working for Debian,
    # OpenSUSE, Fedora, Arch Linux, Raspian, FreeBSD, etc.

    # Add additional PPAs which we need to be able to build cpp-ethereum on
    # Ubuntu Trusty.  That includes our own PPAs, a PPA for getting CMake 3.x
    # on Trusty, and the repository for LLVM.
    sudo add-apt-repository -y ppa:ethereum/ethereum
    sudo add-apt-repository -y ppa:ethereum/ethereum-dev
    sudo apt-add-repository -y ppa:george-edison55/cmake-3.x
    sudo add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main"
    wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
    sudo apt-get -y update

    # Install binaries for nearly all of our dependencies
    sudo apt-get -y install \
        build-essential \
        cmake \
        git \
        libboost-all-dev \
        libcurl4-openssl-dev \
        libjsoncpp-dev \

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
