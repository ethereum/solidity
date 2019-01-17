.. index:: ! installing

.. _installing-solidity:

################################
Installing the Solidity Compiler
################################

Versioning
==========

Solidity versions follow `semantic versioning <https://semver.org>`_ and in addition to
releases, **nightly development builds** are also made available.  The nightly builds
are not guaranteed to be working and despite best efforts they might contain undocumented
and/or broken changes. We recommend using the latest release. Package installers below
will use the latest release.

Remix
=====

*We recommend Remix for small contracts and for quickly learning Solidity.*

`Access Remix online <https://remix.ethereum.org/>`_, you don't need to install anything.
If you want to use it without connection to the Internet, go to
https://github.com/ethereum/remix-live/tree/gh-pages and download the ``.zip`` file as
explained on that page.

Further options on this page detail installing commandline Solidity compiler software
on your computer. Choose a commandline compiler if you are working on a larger contract
or if you require more compilation options.

.. _solcjs:

npm / Node.js
=============

Use `npm` for a convenient and portable way to install `solcjs`, a Solidity compiler. The
`solcjs` program has fewer features than the ways to access the compiler described
further down this page. The
:ref:`commandline-compiler` documentation assumes you are using
the full-featured compiler, `solc`. The usage of `solcjs` is documented inside its own
`repository <https://github.com/ethereum/solc-js>`_.

Note: The solc-js project is derived from the C++
`solc` by using Emscripten which means that both use the same compiler source code.
`solc-js` can be used in JavaScript projects directly (such as Remix).
Please refer to the solc-js repository for instructions.

.. code-block:: bash

    npm install -g solc

.. note::

    The commandline executable is named `solcjs`.

    The comandline options of `solcjs` are not compatible with `solc` and tools (such as `geth`)
    expecting the behaviour of `solc` will not work with `solcjs`.

Docker
======

We provide up to date docker builds for the compiler. The ``stable``
repository contains released versions while the ``nightly``
repository contains potentially unstable changes in the develop branch.

.. code-block:: bash

    docker run ethereum/solc:stable --version

Currently, the docker image only contains the compiler executable,
so you have to do some additional work to link in the source and
output directories.

Binary Packages
===============

Binary packages of Solidity are available at
`solidity/releases <https://github.com/ethereum/solidity/releases>`_.

We also have PPAs for Ubuntu, you can get the latest stable
version using the following commands:

.. code-block:: bash

    sudo add-apt-repository ppa:ethereum/ethereum
    sudo apt-get update
    sudo apt-get install solc

The nightly version can be installed using these commands:

.. code-block:: bash

    sudo add-apt-repository ppa:ethereum/ethereum
    sudo add-apt-repository ppa:ethereum/ethereum-dev
    sudo apt-get update
    sudo apt-get install solc

We are also releasing a `snap package <https://snapcraft.io/>`_, which is installable in all the `supported Linux distros <https://snapcraft.io/docs/core/install>`_. To install the latest stable version of solc:

.. code-block:: bash

    sudo snap install solc

If you want to help testing the latest development version of Solidity
with the most recent changes, please use the following:

.. code-block:: bash

    sudo snap install solc --edge

Arch Linux also has packages, albeit limited to the latest development version:

.. code-block:: bash

    pacman -S solidity

We distribute the Solidity compiler through Homebrew
as a build-from-source version. Pre-built bottles are
currently not supported.

.. code-block:: bash

    brew update
    brew upgrade
    brew tap ethereum/ethereum
    brew install solidity

If you need a specific version of Solidity you can install a
Homebrew formula directly from Github.

View
`solidity.rb commits on Github <https://github.com/ethereum/homebrew-ethereum/commits/master/solidity.rb>`_.

Follow the history links until you have a raw file link of a
specific commit of ``solidity.rb``.

Install it using ``brew``:

.. code-block:: bash

    brew unlink solidity
    # Install 0.4.8
    brew install https://raw.githubusercontent.com/ethereum/homebrew-ethereum/77cce03da9f289e5a3ffe579840d3c5dc0a62717/solidity.rb

Gentoo Linux also provides a solidity package that can be installed using ``emerge``:

.. code-block:: bash

    emerge dev-lang/solidity

.. _building-from-source:

Building from Source
====================

Prerequisites - Linux
---------------------

You need to install the following dependencies for Linux builds of Solidity:

+-----------------------------------+-------------------------------------------------------+
| Software                          | Notes                                                 |
+===================================+=======================================================+
| `Git for Linux`_                  | Command-line tool for retrieving source from Github.  |
+-----------------------------------+-------------------------------------------------------+

.. _Git for Linux: https://git-scm.com/download/linux

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

.. code-block:: bash

    sudo xcodebuild -license accept

Our OS X builds require you to `install the Homebrew <http://brew.sh>`_
package manager for installing external dependencies.
Here's how to `uninstall Homebrew
<https://github.com/Homebrew/homebrew/blob/master/share/doc/homebrew/FAQ.md#how-do-i-uninstall-homebrew>`_,
if you ever want to start again from scratch.


Prerequisites - Windows
-----------------------

You need to install the following dependencies for Windows builds of Solidity:

+-----------------------------------+-------------------------------------------------------+
| Software                          | Notes                                                 |
+===================================+=======================================================+
| `Git for Windows`_                | Command-line tool for retrieving source from Github.  |
+-----------------------------------+-------------------------------------------------------+
| `CMake`_                          | Cross-platform build file generator.                  |
+-----------------------------------+-------------------------------------------------------+
| `Visual Studio 2017 Build Tools`_ | C++ compiler                                          |
+-----------------------------------+-------------------------------------------------------+
| `Visual Studio 2017`_  (Optional) | C++ compiler and dev environment.                     |
+-----------------------------------+-------------------------------------------------------+

If you've already had one IDE and only need compiler and libraries,
you could install Visual Studio 2017 Build Tools.

Visual Studio 2017 provides both IDE and necessary compiler and libraries.
So if you have not got an IDE and prefer to develop solidity, Visual Studio 2017
may be an choice for you to get everything setup easily.

Here is the list of components that should be installed
in Visual Studio 2017 Build Tools or Visual Studio 2017:

* Visual Studio C++ core features
* VC++ 2017 v141 toolset (x86,x64)
* Windows Universal CRT SDK
* Windows 8.1 SDK
* C++/CLI support

.. _Git for Windows: https://git-scm.com/download/win
.. _CMake: https://cmake.org/download/
.. _Visual Studio 2017: https://www.visualstudio.com/vs/
.. _Visual Studio 2017 Build Tools: https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017

Clone the Repository
--------------------

To clone the source code, execute the following command:

.. code-block:: bash

    git clone --recursive https://github.com/ethereum/solidity.git
    cd solidity

If you want to help developing Solidity,
you should fork Solidity and add your personal fork as a second remote:

.. code-block:: bash

    git remote add personal git@github.com:[username]/solidity.git

External Dependencies
---------------------

We have a helper script which installs all required external dependencies
on macOS, Windows and on numerous Linux distros.

.. code-block:: bash

    ./scripts/install_deps.sh

Or, on Windows:

.. code-block:: bat

    scripts\install_deps.bat


Command-Line Build
------------------

**Be sure to install External Dependencies (see above) before build.**

Solidity project uses CMake to configure the build.
You might want to install ccache to speed up repeated builds.
CMake will pick it up automatically.
Building Solidity is quite similar on Linux, macOS and other Unices:

.. code-block:: bash

    mkdir build
    cd build
    cmake .. && make

or even easier:

.. code-block:: bash

    #note: this will install binaries solc and soltest at usr/local/bin
    ./scripts/build.sh

And for Windows:

.. code-block:: bash

    mkdir build
    cd build
    cmake -G "Visual Studio 15 2017 Win64" ..

This latter set of instructions should result in the creation of
**solidity.sln** in that build directory.  Double-clicking on that file
should result in Visual Studio firing up.  We suggest building
**Release** configuration, but all others work.

Alternatively, you can build for Windows on the command-line, like so:

.. code-block:: bash

    cmake --build . --config Release

CMake options
=============

If you are interested what CMake options are available run ``cmake .. -LH``.

.. _smt_solvers_build:

SMT Solvers
-----------
Solidity can be built against SMT solvers and will do so by default if
they are found in the system. Each solver can be disabled by a `cmake` option.

*Note: In some cases, this can also be a potential workaround for build failures.*


Inside the build folder you can disable them, since they are enabled by default:

.. code-block:: bash

    # disables only Z3 SMT Solver.
    cmake .. -DUSE_Z3=OFF

    # disables only CVC4 SMT Solver.
    cmake .. -DUSE_CVC4=OFF

    # disables both Z3 and CVC4
    cmake .. -DUSE_CVC4=OFF -DUSE_Z3=OFF

The version string in detail
============================

The Solidity version string contains four parts:

- the version number
- pre-release tag, usually set to ``develop.YYYY.MM.DD`` or ``nightly.YYYY.MM.DD``
- commit in the format of ``commit.GITHASH``
- platform, which has an arbitrary number of items, containing details about the platform and compiler

If there are local modifications, the commit will be postfixed with ``.mod``.

These parts are combined as required by Semver, where the Solidity pre-release tag equals to the Semver pre-release
and the Solidity commit and platform combined make up the Semver build metadata.

A release example: ``0.4.8+commit.60cc1668.Emscripten.clang``.

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
