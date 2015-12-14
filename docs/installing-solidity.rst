###################
Installing Solidity
###################

Browser-Solidity
================

If you just want to try Solidity for small contracts, you
can try `browser-solidity <https://chriseth.github.io/browser-solidity>`_
which does not need any installation. If you want to use it
without connection to the Internet, you can also just save the page
locally or clone http://github.com/chriseth/browser-solidity.

NPM / node.js
=============

This is probably the most portable and most convenient way to install Solidity locally.

A platform-independent JavaScript library is provided by compiling the C++ source
into JavaScript using Emscripten for browser-solidity and there is also an NPM
package available.

To install it, simply use

::

    npm install solc

Details about the usage of the nodejs package can be found in the
`repository <https://github.com/chriseth/browser-solidity#nodejs-usage>`_.

Binary Packages
===============

Binary packages of Solidity together with its IDE Mix are available through
the `C++ bundle <https://github.com/ethereum/webthree-umbrella/releases>`_ of
Ethereum.

Building from Source
====================

Building Solidity is quite similar on MacOS X, Ubuntu and probably other Unices.
This guide starts explaining how to install the dependencies for each platform
and then shows how to build Solidity itself.

MacOS X
-------


Requirements:

- OS X Yosemite (10.10.5)
- Homebrew
- Xcode

Set up Homebrew:

.. code-block:: bash

    brew update
    brew install boost --c++11             # this takes a while
    brew install cmake cryptopp miniupnpc leveldb gmp libmicrohttpd libjson-rpc-cpp 
    # For Mix IDE and Alethzero only
    brew install xz d-bus
    brew install llvm --HEAD --with-clang 
    brew install qt5 --with-d-bus          # add --verbose if long waits with a stale screen drive you crazy as well

Ubuntu
------

Below are the build instructions for the latest versions of Ubuntu. The best
supported platform as of December 2014 is Ubuntu 14.04, 64 bit, with at least 2
GB RAM. All our tests are done with this version. Community contributions for
other versions are welcome!

Install dependencies:

Before you can build the source, you need several tools and dependencies for the application to get started.

First, update your repositories. Not all packages are provided in the main
Ubuntu repository, those you'll get from the Ethereum PPA and the LLVM archive.

.. note::

    Ubuntu 14.04 users, you'll need the latest version of cmake. For this, use:
    `sudo apt-add-repository ppa:george-edison55/cmake-3.x`

Now add all the rest:

.. code-block:: bash

    sudo apt-get -y update
    sudo apt-get -y install language-pack-en-base
    sudo dpkg-reconfigure locales
    sudo apt-get -y install software-properties-common
    sudo add-apt-repository -y ppa:ethereum/ethereum
    sudo add-apt-repository -y ppa:ethereum/ethereum-dev
    sudo apt-get -y update
    sudo apt-get -y upgrade

Use the following command to add the develop packages:

.. code-block:: bash

    sudo apt-get -y install build-essential git cmake libboost-all-dev libgmp-dev libleveldb-dev libminiupnpc-dev libreadline-dev libncurses5-dev libcurl4-openssl-dev libcryptopp-dev libjson-rpc-cpp-dev libmicrohttpd-dev libjsoncpp-dev libedit-dev libz-dev

Building
--------

Run this if you plan on installing Solidity only, ignore errors at the end as
they relate only to Alethzero and Mix

.. code-block:: bash

    git clone --recursive https://github.com/ethereum/webthree-umbrella.git
    cd webthree-umbrella
    ./webthree-helpers/scripts/ethupdate.sh --no-push --simple-pull --project solidity # update Solidity repo
    ./webthree-helpers/scripts/ethbuild.sh --no-git --project solidity --all --cores 4 # build Solidity

If you opted to install Alethzero and Mix:

.. code-block:: bash

    git clone --recursive https://github.com/ethereum/webthree-umbrella.git
    cd webthree-umbrella && mkdir -p build && cd build
    cmake ..

If you want to help developing Solidity,
you should fork Solidity and add your personal fork as a second remote:

.. code-block:: bash

    cd webthree-umbrella/solidity
    git remote add personal git@github.com:username/solidity.git

Note that webthree-umbrella uses submodules, so solidity is its own git
repository, but its settings are not stored in `.git/config`, but in
`webthree-umbrella/.git/modules/solidity/config`.


