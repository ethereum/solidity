###################
Installing Solidity
###################

Browser-Solidity
================

If you just want to try Solidity for small contracts, you
can try `browser-solidity <https://ethereum.github.io/browser-solidity>`_
which does not need any installation. If you want to use it
without connection to the Internet, you can also just save the page
locally or clone http://github.com/ethereum/browser-solidity.

npm / Node.js
=============

This is probably the most portable and most convenient way to install Solidity locally.

A platform-independent JavaScript library is provided by compiling the C++ source
into JavaScript using Emscripten for browser-solidity and there is also an npm
package available.

To install it, simply use

::

    npm install solc

Details about the usage of the Node.js package can be found in the
`solc-js repository <https://github.com/ethereum/solc-js>`_.

Binary Releases
===============

The `solidity-v0.3.6 <https://github.com/ethereum/solidity/releases/tag/v0.3.6>`_
release (10th August 2016) was the first "standalone" version of Solidity.
All runtime dependencies onto cpp-ethereum have been removed.  This was a pretty
major refactoring.  We also transitioned from Jenkins to TravisCI and Appveyor.

At the time of writing, our Homebrew and ZIP workflows have not yet been
restored.  They will be back for the pending solidity-v0.4.0 release.

Ubuntu PPAs **are** working, though there are still some clashes with the
cpp-ethereum PPAs which won't be resolved until the next cpp-ethereum release.
This is the sequence of commands which should be successful.  We will get the
PPAs into a state where they can cleanly coexist as soon as we can. ::

    sudo add-apt-repository -y ppa:ethereum/ethereum
    sudo add-apt-repository -y ppa:ethereum/ethereum-dev
    apt-get remove libethereum solc cpp-ethereum
    apt-get install solc


Building from Source (macOS, Debian or Ubuntu)
==============================================

As of August 2016 we now support an **install_deps.sh** script which encodes all of the
distro-specific installation steps for macOS, Debian and Ubuntu, with support for more
distros being added all the time.

So the build process is very trivial for these OSes:

.. code-block:: bash

    git clone --recursive https://github.com/ethereum/solidity.git
    cd solidity
    ./scripts/install_deps.sh
    mkdir build
    cd build
    cmake .. && make

Building from Source (Windows)
======================================

Pre-requisites
--------------------------------------------------------------------------------

You will need to install the following dependencies

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

From there, the build process is very similar to macOS and Linux:

.. code-block:: bash

    git clone --recursive https://github.com/ethereum/solidity.git
    cd solidity
    scripts\install_deps.bat
    mkdir build
    cd build
    cmake -G "Visual Studio 14 2015 Win64" ..

And then open the generated **cpp-ethereum.sln** file in Visual Studio, where you can
build and debug it to your hearts content.
