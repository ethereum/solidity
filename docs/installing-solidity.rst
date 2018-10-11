.. index:: ! installing

.. _installing-solidity:

#################################
Installer le Compilateur Solidity
#################################

Versionnage
===========

Les versions de Solidity suivent un `versionnage sémantique <https://semver.org>`_ et en plus des versions stables, des versions de développement **nightly** sont également disponibles.  Les versions nightly ne sont pas garanties de fonctionner et malgré tous les efforts, elles peuvent contenir des changements non documentés et/ou cassés. Nous vous recommandons d'utiliser la dernière version. Les installateurs de paquets ci-dessous
utilisent la dernière version.

Remix
=====

Nous recommandons Remix pour les petits contrats et pour l'apprentissage rapide de Solidity.

`Accédez à Remix en ligne <https://remix.ethereum.org/>`_, vous n'avez rien à installer.
Si vous voulez l'utiliser sans connexion à Internet, allez à
https://github.com/ethereum/remix-live/tree/gh-pages et téléchargez le fichier ``.zip`` tel qu'expliqué sur cette page.

D'autres options sur cette page détaillent l'installation du compilateur Solidity en ligne de commande
sur votre ordinateur. Choisissez un compilateur de ligne de commande si vous travaillez sur un contrat plus important
ou si vous avez besoin de plus d'options de compilation.

.. _solcjs:

npm / Node.js
=============

Utilisez `npm' pour un moyen pratique et portable d'installer `solcjs', un compilateur Solidity. Le programme `solcjs` a moins de fonctionnalités que le  compilateur décrit plus bas sur cette page. La documentation du :ref:`commandline-compiler` suppose que vous utilisez le compilateur complet, `solc`. L'utilisation de `solcjs` est documentée dans son propre
`dépot <https://github.com/ethereum/solc-js>`_.

Note : Le projet solc-js est dérivé du projet C++ `solc` en utilisant Emscripten, ce qui signifie que les deux utilisent le même code source du compilateur.
`solc-js' peut être utilisé directement dans les projets JavaScript (comme Remix).
Veuillez vous référer au dépôt solc-js pour les instructions.

.. code-block:: bash

    npm install -g solc

.. note::

    L'exécutable en ligne de commande est nommé `solcjs'.

    Les options de la ligne de commande de `solcjs' ne sont pas compatibles avec `solc' et les outils (tels que `geth') attendant le comportement de `solc' ne fonctionneront pas avec `solcjs'.

Docker
======

Nous fournissons des images dockers à jour pour le compilateur. Le dépot ``stable``
contient les versions publiées tandis que le dépôt ``nightly`` contient des changements potentiellement instables dans la branche develop.

.. code-block:: bash

    docker run ethereum/solc:stable --version

Actuellement, l'image du docker ne contient que l'exécutable du compilateur,
donc vous devez faire un peu plus de travail pour lier le code source et
répertoires de sortie.

Paquets binaires
================

Les binaires de Solidity sont disponibles à
`solidity/releases <https://github.com/ethereum/solidity/releases>`_.

Nous avons également des PPAs for Ubuntu, vous pouvez obtenir la dernière version via la commande:

.. code-block:: bash

    sudo add-apt-repository ppa:ethereum/ethereum
    sudo apt-get update
    sudo apt-get install solc

La version nigthly peut s'installer avec la commande:

.. code-block:: bash

    sudo add-apt-repository ppa:ethereum/ethereum
    sudo add-apt-repository ppa:ethereum/ethereum-dev
    sudo apt-get update
    sudo apt-get install solc

Nous publions également un `package snap <https://snapcraft.io/>`_, installable dans toutes les `distributionss linux supportées <https://snapcraft.io/docs/core/install>`_. Pour installer la dernière evrsion stable de solc:

.. code-block:: bash

    sudo snap install solc

Si vous voulez aider aux tests en utilisant la dernière version de développement, avec les changements l;es plus récents, merci d'utiliser:

.. code-block:: bash

    sudo snap install solc --edge

Arch Linux a aussi des packets, bien que limités à la dernière version de développement:

.. code-block:: bash

    pacman -S solidity

Nous distribuons également le compilateur Solidity via homebrew dans une version compilée à partir des sources. Les "bottles" pré-compilées ne sont pas encore supportées pour l'instant.

.. code-block:: bash

    brew update
    brew upgrade
    brew tap ethereum/ethereum
    brew install solidity

Si vous avec besoin d'une version spécifique, vous pouvez exécuter la formule homebrew correspondante disponible sur GitHub.

Regarder
`commits de solidity.rb sur Github <https://github.com/ethereum/homebrew-ethereum/commits/master/solidity.rb>`_.

Suivez l'historique des liens jusqu'à avoir un lien de ficher brut ("raw")
d'un commit spécifique de ``solidity.rb``.

Installez-le via ``brew``:

.. code-block:: bash

    brew unlink solidity
    # Install 0.4.8
    brew install https://raw.githubusercontent.com/ethereum/homebrew-ethereum/77cce03da9f289e5a3ffe579840d3c5dc0a62717/solidity.rb

Gentoo Linux dispose aussi d' un paquet Solidity installable via ``emerge``:

.. code-block:: bash

    emerge dev-lang/solidity

.. _building-from-source:

Compilation à partir des sources
================================

Prérequis - Linux
-----------------

Vous aurez besoin des dépendances suivantes pour les compilations de Solidity sous Linux:

+-----------------------------------+-----------------------------------------------------------------------+
| Software                          | Notes                                                                 |
+===================================+=======================================================================+
| `Git pour Linux`_                  | Outils en ligne de commande pour r'ecup'erer des fichiers sur github  |
+-----------------------------------+-----------------------------------------------------------------------+

.. _Git pour Linux: https://git-scm.com/download/linux

Prérequis - macOS
-----------------

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

Solidity has git submodules.  Ensure they are properly loaded:

.. code-block:: bash

   git submodule update --init --recursive

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
**RelWithDebugInfo** configuration, but all others work.

Alternatively, you can build for Windows on the command-line, like so:

.. code-block:: bash

    cmake --build . --config RelWithDebInfo

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
