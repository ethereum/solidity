.. index:: ! installing

.. _installing-solidity:

###################
Installing Solidity
###################

Versioning
==========

Solidity versions follow `semantic versioning <https://semver.org>`_ and in addition to
releases, **nightly development builds** are also made available.  The nightly builds
are not guaranteed to be working and despite best efforts they might contain undocumented
and/or broken changes. We recommend using the latest release. Package installers below
will use the latest release.

Remix
=====

If you just want to try Solidity for small contracts, you
can try `Remix <https://remix.ethereum.org/>`_
which does not need any installation. If you want to use it
without connection to the Internet, you can go to
https://github.com/ethereum/browser-solidity/tree/gh-pages and
download the .ZIP file as explained on that page.

npm / Node.js
=============

This is probably the most portable and most convenient way to install Solidity locally.

A platform-independent JavaScript library is provided by compiling the C++ source
into JavaScript using Emscripten. It can be used in projects directly (such as Remix).
Please refer to the `solc-js <https://github.com/ethereum/solc-js>`_ repository for instructions.

It also contains a commandline tool called `solcjs`, which can be installed via npm:

.. code:: bash

    npm install -g solc

.. note::

    The comandline options of `solcjs` are not compatible with `solc` and tools (such as `geth`)
    expecting the behaviour of `solc` will not work with `solcjs`.

Docker
======

We provide up to date docker builds for the compiler. The ``stable``
repository contains released versions while the ``nightly``
repository contains potentially unstable changes in the develop branch.

.. code:: bash

    docker run ethereum/solc:stable solc --version

Currenty, the docker image only contains the compiler executable,
so you have to do some additional work to link in the source and
output directories.

Binary Packages
===============

Binary packages of Solidity available at
`solidity/releases <https://github.com/ethereum/solidity/releases>`_.

We also have PPAs for Ubuntu.  For the latest stable version.

.. code:: bash

    sudo add-apt-repository ppa:ethereum/ethereum
    sudo apt-get update
    sudo apt-get install solc

If you want to use the cutting edge developer version:

.. code:: bash

    sudo add-apt-repository ppa:ethereum/ethereum
    sudo add-apt-repository ppa:ethereum/ethereum-dev
    sudo apt-get update
    sudo apt-get install solc

Arch Linux also has packages, albeit limited to the latest development version:

.. code:: bash

    pacman -S solidity-git

Homebrew is missing pre-built bottles at the time of writing,
following a Jenkins to TravisCI migration, but Homebrew
should still work just fine as a means to build-from-source.
We will re-add the pre-built bottles soon.

.. code:: bash

    brew update
    brew upgrade
    brew tap ethereum/ethereum
    brew install solidity
    brew linkapps solidity

If you need a specific version of Solidity you can install a 
Homebrew formula directly from Github.

View 
`solidity.rb commits on Github <https://github.com/ethereum/homebrew-ethereum/commits/master/solidity.rb>`_.

Follow the history links until you have a raw file link of a 
specific commit of ``solidity.rb``.

Install it using ``brew``:

.. code:: bash

    brew unlink solidity
    # Install 0.4.8
    brew install https://raw.githubusercontent.com/ethereum/homebrew-ethereum/77cce03da9f289e5a3ffe579840d3c5dc0a62717/solidity.rb

.. _building-from-source:

Building from Source
====================

Clone the Repository
--------------------

To clone the source code, execute the following command:

.. code:: bash

    git clone --recursive https://github.com/ethereum/solidity.git
    cd solidity

If you want to help developing Solidity,
you should fork Solidity and add your personal fork as a second remote:

.. code:: bash

    cd solidity
    git remote add personal git@github.com:[username]/solidity.git

Solidity has git submodules.  Ensure they are properly loaded:

.. code:: bash

   git submodule update --init --recursive

Prerequisites - macOS
---------------------

For macOS, ensure that you have the latest version of
`Xcode installed <https://developer.apple.com/xcode/download/>`_.
This contains the `Clang C++ compiler <https://en.wikipedia.org/wiki/Clang>`_, the
`Xcode IDE <https://en.wikipedia.org/wiki/Xcode>`_ and other Apple development
tools which are required for building C++ applications on OS X.
If you are installing Xcode for the first time, or have just installed a new
version then you will need to agree to the license before you can do
command-line builds:

.. code:: bash

    sudo xcodebuild -license accept

Our OS X builds require you to `install the Homebrew <http://brew.sh>`_
package manager for installing external dependencies.
Here's how to `uninstall Homebrew
<https://github.com/Homebrew/homebrew/blob/master/share/doc/homebrew/FAQ.md#how-do-i-uninstall-homebrew>`_,
if you ever want to start again from scratch.


Prerequisites - Windows
-----------------------

You will need to install the following dependencies for Windows builds of Solidity:

+------------------------------+-------------------------------------------------------+
| Software                     | Notes                                                 |
+==============================+=======================================================+
| `Git for Windows`_           | Command-line tool for retrieving source from Github.  |
+------------------------------+-------------------------------------------------------+
| `CMake`_                     | Cross-platform build file generator.                  |
+------------------------------+-------------------------------------------------------+
| `Visual Studio 2015`_        | C++ compiler and dev environment.                     |
+------------------------------+-------------------------------------------------------+

.. _Git for Windows: https://git-scm.com/download/win
.. _CMake: https://cmake.org/download/
.. _Visual Studio 2015: https://www.visualstudio.com/products/vs-2015-product-editions


External Dependencies
---------------------

We now have a "one button" script which installs all required external dependencies
on macOS, Windows and on numerous Linux distros.  This used to be a multi-step
manual process, but is now a one-liner:

.. code:: bash

    ./scripts/install_deps.sh

Or, on Windows:

.. code:: bat

    scripts\install_deps.bat


Command-Line Build
------------------

Building Solidity is quite similar on Linux, macOS and other Unices:

.. code:: bash

    mkdir build
    cd build
    cmake .. && make

or even easier:

.. code:: bash
    
    #note: this will install binaries solc and soltest at usr/local/bin
    ./scripts/build.sh

And even for Windows:

.. code:: bash

    mkdir build
    cd build
    cmake -G "Visual Studio 14 2015 Win64" ..

This latter set of instructions should result in the creation of
**solidity.sln** in that build directory.  Double-clicking on that file
should result in Visual Studio firing up.  We suggest building
**RelWithDebugInfo** configuration, but all others work.

Alternatively, you can build for Windows on the command-line, like so:

.. code:: bash

    cmake --build . --config RelWithDebInfo

The version string in detail
============================

The Solidity version string contains four parts:

- the version number
- pre-release tag, usually set to ``develop.YYYY.MM.DD`` or ``nightly.YYYY.MM.DD``
- commit in the format of ``commit.GITHASH``
- platform has arbitrary number of items, containing details about the platform and compiler

If there are local modifications, the commit will be postfixed with ``.mod``.

These parts are combined as required by Semver, where the Solidity pre-release tag equals to the Semver pre-release
and the Solidity commit and platform combined make up the Semver build metadata.

A relase example: ``0.4.8+commit.60cc1668.Emscripten.clang``.

A pre-release example: ``0.4.9-nightly.2017.1.17+commit.6ecb4aa3.Emscripten.clang``

Important information about versioning
======================================

After a release is made, the patch version level is bumped, because we assume that only
patch level changes follow. When changes are merged, the version should be bumped according
to semver and the severity of the change. Finally, a release is always made with the version
of the current nightly build, but without the ``prerelease`` specifier.

Example:

0. the 0.4.0 release is made
1. nightly build has a version of 0.4.1 from now on
2. non-breaking changes are introduced - no change in version
3. a breaking change is introduced - version is bumped to 0.5.0
4. the 0.5.0 release is made

This behaviour works well with the  :ref:`version pragma <version_pragma>`.
