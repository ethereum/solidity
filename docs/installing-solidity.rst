###################
Installing Solidity
###################

Browser-Solidity
================

If you just want to try Solidity for small contracts, you
can try `browser-solidity <https://ethereum.github.io/browser-solidity>`_
which does not need any installation. If you want to use it
without connection to the Internet, you can go to
https://github.com/ethereum/browser-solidity/tree/gh-pages and
download the .ZIP file as explained on that page.


npm / Node.js
=============

This is probably the most portable and most convenient way to install Solidity locally.

A platform-independent JavaScript library is provided by compiling the C++ source
into JavaScript using Emscripten for browser-solidity and there is also an npm
package available.

To install it, simply use

.. code:: bash

    npm install solc

Details about the usage of the Node.js package can be found in the
`solc-js repository <https://github.com/ethereum/solc-js>`_.

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
    git remote add personal git@github.com:\<username\>/solidity.git


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

And even on Windows:

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
