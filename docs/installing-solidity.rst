.. index:: ! installing

.. _installing-solidity:

#################################
Installer le Compilateur Solidity
#################################

Versionnage
===========

Les versions de Solidity suivent un `versionnage sémantique <https://semver.org>`_ et en plus des versions stables, des versions de développement **nightly development builds** sont également disponibles.  Les versions nightly ne sont pas garanties de fonctionner et malgré tous les efforts, elles peuvent contenir des changements non documentés et/ou cassés. Nous vous recommandons d'utiliser la dernière version. Les installateurs de paquets ci-dessous
utilisent la dernière version.

Remix
=====

Nous recommandons Remix pour les petits contrats et pour l'apprentissage rapide de Solidity.

`Accédez à Remix en ligne <https://remix.ethereum.org/>`_, vous n'avez rien à installer.
Si vous voulez l'utiliser sans connexion à Internet, allez à
https://github.com/ethereum/remix-live/tree/gh-pages et téléchargez le fichier ``.zip`` tel qu'expliqué sur cette page. Remix is also a convenient option for testing nightly builds
without installing multiple Solidity versions.

D'autres options sur cette page détaillent l'installation du compilateur Solidity en ligne de commande
sur votre ordinateur. Choisissez un compilateur de ligne de commande si vous travaillez sur un contrat plus important
ou si vous avez besoin de plus d'options de compilation.

.. _solcjs:

npm / Node.js
=============

Utilisez `npm' pour un moyen pratique et portable d'installer `solcjs', un compilateur Solidity. Le programme `solcjs` a moins de fonctionnalités que le  compilateur décrit plus bas sur cette page. La documentation du :ref:`commandline-compiler` suppose que vous utilisez le compilateur complet, `solc`. L'utilisation de `solcjs` est documentée dans son propre
`dépot <https://github.com/ethereum/solc-js>`_.

Note : Le projet solc-js est dérivé du projet C++ `solc` en utilisant Emscripten, ce qui signifie que les deux utilisent le même code source du compilateur.
`solc-js` peut être utilisé directement dans les projets JavaScript (comme Remix).
Veuillez vous référer au dépôt solc-js pour les instructions.

.. code-block:: bash

    npm install -g solc

.. note::

    L'exécutable en ligne de commande est nommé `solcjs`.

    Les options de la ligne de commande de `solcjs` ne sont pas compatibles avec `solc' et les outils (tels que `geth') attendant le comportement de `solc` ne fonctionneront pas avec `solcjs`.


Docker
======

Nous fournissons des images dockers à jour pour le compilateur via l'image ``solc`` distribué par l'organisation ``ethereum``. Le label ``stable``
contient les versions publiées tandis que le label ``nightly`` contient des changements potentiellement instables dans la branche develop.
Docker images of Solidity builds are available using the ``solc`` image from the ``ethereum`` organisation.
Use the ``stable`` tag for the latest released version, and ``nightly`` for potentially unstable changes in the develop branch.

The Docker image runs the compiler executable, so you can pass all compiler arguments to it.
For example, the command below pulls the stable version of the ``solc`` image (if you do not have it already),
and runs it in a new container, passing the ``--help`` argument.

.. code-block:: bash

    docker run ethereum/solc:stable --help

You can also specify release build versions in the tag, for example, for the 0.5.4 release.

.. code-block:: bash

    docker run ethereum/solc:0.5.4 --help

To use the Docker image to compile Solidity files on the host machine mount a
local folder for input and output, and specify the contract to compile. For example.

.. code-block:: bash

    docker run -v /local/path:/sources ethereum/solc:stable -o /sources/output --abi --bin /sources/Contract.sol

You can also use the standard JSON interface (which is recommended when using the compiler with tooling).
When using this interface it is not necessary to mount any directories.

.. code-block:: bash

    docker run ethereum/solc:stable --standard-json < input.json > output.json

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

.. note::

    The ``solc`` snap uses strict confinement. This is the most secure mode for snap packages
    but it comes with limitations, like accessing only the files in your ``/home`` and ``/media`` directories.
    For more information, go to `Demystifying Snap Confinement <https://snapcraft.io/blog/demystifying-snap-confinement>`_.

Arch Linux a aussi des paquets, bien que limités à la dernière version de développement:

.. code-block:: bash

    pacman -S solidity

Nous distribuons également le compilateur Solidity via homebrew dans une version compilée à partir des sources. Les "bottles" pré-compilées ne sont pas encore supportées pour l'instant.

.. code-block:: bash

    brew update
    brew upgrade
    brew tap ethereum/ethereum
    brew install solidity

To install the most recent 0.4.x / 0.5.x version of Solidity you can also use ``brew install solidity@4``
and ``brew install solidity@5``, respectively.

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


Gentoo Linux has an `Ethereum overlay <https://overlays.gentoo.org/#ethereum>`_ that contains a solidity package.
After the overlay is setup, ``solc`` can be installed in x86_64 architectures by:

.. code-block:: bash

    emerge dev-lang/solidity

.. _building-from-source:

Compilation à partir des sources
================================

Prérequis - Linux
-----------------

Vous aurez besoin des dépendances suivantes pour toutes compilations de Solidity:

+-----------------------------------+-------------------------------------------------------+
| Software                          | Notes                                                 |
+===================================+=======================================================+
| `CMake`_ (version 3.9+)           | Cross-platform build file generator.                  |
+-----------------------------------+-------------------------------------------------------+
| `Boost`_  (version 1.65+)         | C++ libraries.                                        |
+-----------------------------------+-------------------------------------------------------+
| `Git`_                            | Command-line tool for retrieving source code.         |
+-----------------------------------+-------------------------------------------------------+
| `z3`_ (version 4.6+, Optional)    | For use with SMT checker.                             |
+-----------------------------------+-------------------------------------------------------+
| `cvc4`_ (Optional)                | For use with SMT checker.                             |
+-----------------------------------+-------------------------------------------------------+

.. _cvc4: http://cvc4.cs.stanford.edu/web/
.. _Git: https://git-scm.com/download
.. _Boost: https://www.boost.org
.. _CMake: https://cmake.org/download/
.. _z3: https://github.com/Z3Prover/z3

.. note::
    Solidity versions prior to 0.5.10 can fail to correctly link against Boost versions 1.70+.
    A possible workaround is to temporarily rename ``<Boost install path>/lib/cmake/Boost-1.70.0``
    prior to running the cmake command to configure solidity.

    Starting from 0.5.10 linking against Boost 1.70+ should work without manual intervention.

Minimum compiler versions
^^^^^^^^^^^^^^^^^^^^^^^^^

The following C++ compilers and their minimum versions can build the Solidity codebase:

- `GCC <https://gcc.gnu.org>`_, version 5+
- `Clang <https://clang.llvm.org/>`_, version 3.4+
- `MSVC <https://docs.microsoft.com/en-us/cpp/?view=vs-2019>`_, version 2017+

Prérequis - macOS
-----------------

Pour macOS, assurez-vous d'avoir installer la dernière version de
`Xcode <https://developer.apple.com/xcode/download/>`_.
Ceci contient le compilateur C++ `Clang <https://en.wikipedia.org/wiki/Clang>`_, l'IDE
`Xcode <https://en.wikipedia.org/wiki/Xcode>`_ et d'autres outils de développement Apple qui sont nécessaires pour construire des applications C++ sous OS X.
Si vous installez Xcode pour la première fois, ou si vous venez d'installer une nouvelle version, vous devrez accepter la licence avant de pouvoir compiler en ligne de commande:

.. code-block:: bash

    sudo xcodebuild -license accept

Nos versions pour OS X exigent que vous installiez `Homebrew <http://brew.sh>`_http://brew.sh
pour l'installation des dépendances externes.
Voici comment `désinstaller Homebrew
<https://github.com/Homebrew/homebrew/blob/master/share/doc/homebrew/FAQ.md#how-do-i-uninstall-homebrew>`_,
si vous voulez recommencer à zéro.

Prérequis - Windows
-------------------

Vous aurez besoin des dépendances suivants pour compiler Solidity sous Windows:

+-----------------------------------+-------------------------------------------------------+
| Software                          | Notes                                                 |
+===================================+=======================================================+
| `Visual Studio 2017 Build Tools`_ | C++ compiler                                          |
+-----------------------------------+-------------------------------------------------------+
| `Visual Studio 2017`_  (Optional) | C++ compiler and dev environment.                     |
+-----------------------------------+-------------------------------------------------------+

Si vous avez déjà eu un IDE et que vous n'avez besoin que du compilateur et des bibliothèques,
vous pouvez installer Visual Studio 2017 Build Tools.

Visual Studio 2017 fournit à la fois l'IDE et le compilateur et les bibliothèques nécessaires.
Donc si vous n'avez pas d'IDE et que vous préférez développer en Solidity, Visual Studio 2017
peut être un choix pour tout installer facilement.

Voici la liste des composants à installer
dans Visual Studio 2017 Build Tools ou Visual Studio 2017 :

* Visual Studio C+++ fonctionnalités de base
* VC+++ 2017 v141 toolset (x86,x64)
* Windows Universal CRT SDK
* Windows 8.1 SDK
* Support C+++/CLI

.. _Visual Studio 2017: https://www.visualstudio.com/vs/
.. _Visual Studio 2017 Build Tools: https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017

Dependencies Helper Script
--------------------------

We have a helper script which you can use to install all required external dependencies
on macOS, Windows and on numerous Linux distros.

.. code-block:: bash

    ./scripts/install_deps.sh

Or, on Windows:

.. code-block:: bat

    scripts\install_deps.bat

Clonez le dépot
---------------

Pour cloner le code source, exécutez la commande suivante:

.. code-block:: bash

    git clone --recursive https://github.com/ethereum/solidity.git
    cd solidity

Si vous voulez aider à développer Solidity,
vous devriez forker Solidity et ajouter votre fork comme un second dépot distant:

.. code-block:: bash

    git remote add personal git@github.com:[username]/solidity.git

Compilation en ligne de commande
--------------------------------

**Soyez sûrs d'installer les dépendances externes avant de compiler.**

Le projet Solidity utilise CMake pour la configuration de compilation.
Vous voulez peut-être installer ccache pour accélérer des compilations successives.
CMake l'utilisera automatiquement.
Compiler Solidity est similaire sur Linux, macOS et autres systèmes Unix:


.. code-block:: bash

    mkdir build
    cd build
    cmake .. && make

ou même sous Linux et macOS, vous pouvez:

.. code-block:: bash

    #note: this will install binaries solc and soltest at usr/local/bin
    ./scripts/build.sh

.. warning::

    BSD builds should work, but are untested by the Solidity team.

Et pour Windows:

.. code-block:: bash

    mkdir build
    cd build
    cmake -G "Visual Studio 15 2017 Win64" ..

Ce dernier ensemble d'instructions devrait aboutir à la création de **solidity.sln** dans ce répertoire de compilation.  Double-cliquer sur ce fichier devrait faire démarrer Visual Studio.  Nous suggérons de construire la configuration **RelWithDebugInfo**, mais toutes les autres fonctionnent.

Alternativement, vous pouvez compiler pour Windows en ligne de commande, comme ça :

.. code-block:: bash

    cmake --build . --config Release

Options de CMake
================

La liste des options de Cmake est disponible via la commande: ``cmake .. -LH``.

.. _smt_solvers_build:

Solveurs SMT
------------
Solidity peut être compilé avec les solveurs SMT et le fera par défaut s'ils sont trouvés dans le système. Chaque solveur peut être désactivé par une option `cmake`.

Remarque : Dans certains cas, cela peut également être une solution de contournement potentielle en cas d'échec de compilation.


Dans le dossier de compilation, vous pouvez les désactiver, car ils sont activés par défaut :

.. code-block:: bash

    # désactive seulement Z3 SMT Solver.
    cmake .. -DUSE_Z3=OFF

    # désactive seulement CVC4 SMT Solver.
    cmake .. -DUSE_CVC4=OFF

    # désactive Z3 et CVC4
    cmake .. -DUSE_CVC4=OFF -DUSE_Z3=OFF

La string de version en détail
==============================

La string de version de Solidity contient 4 parties:

- le numéro de version
- la balise de pre-version, généralement définie sur ``develop.YYYY.MM.DD`` ou ``nightly.YYYY.MM.DD``.
- commit au format ``commit.GITHASH``.
- plate-forme, qui a un nombre arbitraire d'éléments, contenant des détails sur la plate-forme et le compilateur

S'il y a des modifications locales, le commit sera suffixé avec ``.mod``.

Ces parties sont combinées comme l'exige Semver, où la balise de pré-version Solidity est identique à la pré-version de Semver.
et le commit Solidity et la plate-forme Solidity combinés constituent les métadonnées de la construction Semver.

Un exemple de version : ```0.4.8+commit.60cc1668.Emscripten.clang``.

Un exemple de pré-version : ``0.4.9-nightly.2017.1.17+commit.6ecb4aaa3.Emscripten.clang``

Informations importantes concernant le versionnage
==================================================

Après la sortie d'une version, la version de correctif est incrémentée, parce que nous supposons que seulement les changements de niveau patch suivent. Lorsque les modifications sont fusionnées, la version doit être supprimée en fonction des éléments suivants et la gravité du changement. Enfin, une version est toujours basée sur la nigthly actuelle, mais sans le spécificateur ``prerelease``.

Exemple :

0. la version 0.4.0 est faite
1. nightly build a une version de 0.4.1 à partir de maintenant
2. des modifications incessantes sont introduites - pas de changement de version
3. un changement de rupture est introduit - la version est augmentée à 0.5.0
4. la version 0.5.0 est faite

Ce comportement fonctionne bien avec le :ref:`version pragma <version_pragma>`.
