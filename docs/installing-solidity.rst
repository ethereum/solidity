.. index:: ! installing

.. _installing-solidity:

#################################
Installer le Compilateur Solidity
#################################

Versionnage
===========

Les versions de Solidity suivent un `versionnage sémantique <https://semver.org>`_. In
addition, patch level releases with major release 0 (i.e. 0.x.y) will not
contain breaking changes. That means code that compiles with version 0.x.y
can be expected to compile with 0.x.z where z > y.

In addition to releases, we provide **nightly development builds** with the
intention of making it easy for developers to try out upcoming features and
provide early feedback. Note, however, that while the nightly builds are usually
very stable, they contain bleeding-edge code from the development branch and are
not guaranteed to be always working. Despite our best efforts, they might
contain undocumented and/or broken changes that will not become a part of an
actual release. They are not meant for production use.

When deploying contracts, you should use the latest released version of Solidity. This
is because breaking changes, as well as new features and bug fixes are introduced regularly.
We currently use a 0.x version number `to indicate this fast pace of change <https://semver.org/#spec-item-4>`_.

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
When using this interface it is not necessary to mount any directories as long as the JSON input is
self-contained (i.e. it does not refer to any external files that would have to be
:ref:`loaded by the import callback <initial-vfs-content-standard-json-with-import-callback>`).

.. code-block:: bash

    docker run ethereum/solc:stable --standard-json < input.json > output.json

Paquets Linux
=============

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

Furthermore, some Linux distributions provide their own packages. These packages are not directly
maintained by us, but usually kept up-to-date by the respective package maintainers.

For example, Arch Linux has packages for the latest development version:

.. code-block:: bash

    pacman -S solidity

There is also a `snap package <https://snapcraft.io/solc>`_, however, it is **currently unmaintained**.
It is installable in all the `supported Linux distros <https://snapcraft.io/docs/core/install>`_. To
install the latest stable version of solc:

.. code-block:: bash

    sudo snap install solc

Si vous voulez aider aux tests en utilisant la dernière version de développement, avec les changements les plus récents, merci d'utiliser:

.. code-block:: bash

    sudo snap install solc --edge

.. note::

    The ``solc`` snap uses strict confinement. This is the most secure mode for snap packages
    but it comes with limitations, like accessing only the files in your ``/home`` and ``/media`` directories.
    For more information, go to `Demystifying Snap Confinement <https://snapcraft.io/blog/demystifying-snap-confinement>`_.


macOS Packages
==============

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

Copy the commit hash of the version you want and check it out on your machine.

.. code-block:: bash

    git clone https://github.com/ethereum/homebrew-ethereum.git
    cd homebrew-ethereum
    git checkout <your-hash-goes-here>

Installez-le via ``brew``:

.. code-block:: bash

    brew unlink solidity
    # eg. Install 0.4.8
    brew install solidity.rb

Static Binaries
===============

We maintain a repository containing static builds of past and current compiler versions for all
supported platforms at `solc-bin`_. This is also the location where you can find the nightly builds.

The repository is not only a quick and easy way for end users to get binaries ready to be used
out-of-the-box but it is also meant to be friendly to third-party tools:

- The content is mirrored to https://binaries.soliditylang.org where it can be easily downloaded over
  HTTPS without any authentication, rate limiting or the need to use git.
- Content is served with correct `Content-Type` headers and lenient CORS configuration so that it
  can be directly loaded by tools running in the browser.
- Binaries do not require installation or unpacking (with the exception of older Windows builds
  bundled with necessary DLLs).
- We strive for a high level of backwards-compatibility. Files, once added, are not removed or moved
  without providing a symlink/redirect at the old location. They are also never modified
  in place and should always match the original checksum. The only exception would be broken or
  unusable files with a potential to cause more harm than good if left as is.
- Files are served over both HTTP and HTTPS. As long as you obtain the file list in a secure way
  (via git, HTTPS, IPFS or just have it cached locally) and verify hashes of the binaries
  after downloading them, you do not have to use HTTPS for the binaries themselves.

The same binaries are in most cases available on the `Solidity release page on Github`_. The
difference is that we do not generally update old releases on the Github release page. This means
that we do not rename them if the naming convention changes and we do not add builds for platforms
that were not supported at the time of release. This only happens in ``solc-bin``.

The ``solc-bin`` repository contains several top-level directories, each representing a single platform.
Each one contains a ``list.json`` file listing the available binaries. For example in
``emscripten-wasm32/list.json`` you will find the following information about version 0.7.4:

.. code-block:: json

    {
      "path": "solc-emscripten-wasm32-v0.7.4+commit.3f05b770.js",
      "version": "0.7.4",
      "build": "commit.3f05b770",
      "longVersion": "0.7.4+commit.3f05b770",
      "keccak256": "0x300330ecd127756b824aa13e843cb1f43c473cb22eaf3750d5fb9c99279af8c3",
      "sha256": "0x2b55ed5fec4d9625b6c7b3ab1abd2b7fb7dd2a9c68543bf0323db2c7e2d55af2",
      "urls": [
        "bzzr://16c5f09109c793db99fe35f037c6092b061bd39260ee7a677c8a97f18c955ab1",
        "dweb:/ipfs/QmTLs5MuLEWXQkths41HiACoXDiH8zxyqBHGFDRSzVE5CS"
      ]
    }

This means that:

- You can find the binary in the same directory under the name
  `solc-emscripten-wasm32-v0.7.4+commit.3f05b770.js <https://github.com/ethereum/solc-bin/blob/gh-pages/emscripten-wasm32/solc-emscripten-wasm32-v0.7.4+commit.3f05b770.js>`_.
  Note that the file might be a symlink, and you will need to resolve it yourself if you are not using
  git to download it or your file system does not support symlinks.
- The binary is also mirrored at https://binaries.soliditylang.org/emscripten-wasm32/solc-emscripten-wasm32-v0.7.4+commit.3f05b770.js.
  In this case git is not necessary and symlinks are resolved transparently, either by serving a copy
  of the file or returning a HTTP redirect.
- The file is also available on IPFS at `QmTLs5MuLEWXQkths41HiACoXDiH8zxyqBHGFDRSzVE5CS`_.
- The file might in future be available on Swarm at `16c5f09109c793db99fe35f037c6092b061bd39260ee7a677c8a97f18c955ab1`_.
- You can verify the integrity of the binary by comparing its keccak256 hash to
  ``0x300330ecd127756b824aa13e843cb1f43c473cb22eaf3750d5fb9c99279af8c3``.  The hash can be computed
  on the command line using ``keccak256sum`` utility provided by `sha3sum`_ or `keccak256() function
  from ethereumjs-util`_ in JavaScript.
- You can also verify the integrity of the binary by comparing its sha256 hash to
  ``0x2b55ed5fec4d9625b6c7b3ab1abd2b7fb7dd2a9c68543bf0323db2c7e2d55af2``.

.. warning::

   Due to the strong backwards compatibility requirement the repository contains some legacy elements
   but you should avoid using them when writing new tools:

   - Use ``emscripten-wasm32/`` (with a fallback to ``emscripten-asmjs/``) instead of ``bin/`` if
     you want the best performance. Until version 0.6.1 we only provided asm.js binaries.
     Starting with 0.6.2 we switched to `WebAssembly builds`_ with much better performance. We have
     rebuilt the older versions for wasm but the original asm.js files remain in ``bin/``.
     The new ones had to be placed in a separate directory to avoid name clashes.
   - Use ``emscripten-asmjs/`` and ``emscripten-wasm32/`` instead of ``bin/`` and ``wasm/`` directories
     if you want to be sure whether you are downloading a wasm or an asm.js binary.
   - Use ``list.json`` instead of ``list.js`` and ``list.txt``. The JSON list format contains all
     the information from the old ones and more.
   - Use https://binaries.soliditylang.org instead of https://solc-bin.ethereum.org. To keep things
     simple we moved almost everything related to the compiler under the new ``soliditylang.org``
     domain and this applies to ``solc-bin`` too. While the new domain is recommended, the old one
     is still fully supported and guaranteed to point at the same location.

.. warning::

    The binaries are also available at https://ethereum.github.io/solc-bin/ but this page
    stopped being updated just after the release of version 0.7.2, will not receive any new releases
    or nightly builds for any platform and does not serve the new directory structure, including
    non-emscripten builds.

    If you are using it, please switch to https://binaries.soliditylang.org, which is a drop-in
    replacement. This allows us to make changes to the underlying hosting in a transparent way and
    minimize disruption. Unlike the ``ethereum.github.io`` domain, which we do not have any control
    over, ``binaries.soliditylang.org`` is guaranteed to work and maintain the same URL structure
    in the long-term.

.. _IPFS: https://ipfs.io
.. _Swarm: https://swarm-gateways.net/bzz:/swarm.eth
.. _solc-bin: https://github.com/ethereum/solc-bin/
.. _Solidity release page on github: https://github.com/ethereum/solidity/releases
.. _sha3sum: https://github.com/maandree/sha3sum
.. _keccak256() function from ethereumjs-util: https://github.com/ethereumjs/ethereumjs-util/blob/master/docs/modules/_hash_.md#const-keccak256
.. _WebAssembly builds: https://emscripten.org/docs/compiling/WebAssembly.html
.. _QmTLs5MuLEWXQkths41HiACoXDiH8zxyqBHGFDRSzVE5CS: https://gateway.ipfs.io/ipfs/QmTLs5MuLEWXQkths41HiACoXDiH8zxyqBHGFDRSzVE5CS
.. _16c5f09109c793db99fe35f037c6092b061bd39260ee7a677c8a97f18c955ab1: https://swarm-gateways.net/bzz:/16c5f09109c793db99fe35f037c6092b061bd39260ee7a677c8a97f18c955ab1/

.. _building-from-source:

Compilation à partir des sources
================================

Prérequis - Linux
-----------------

Vous aurez besoin des dépendances suivantes pour toutes compilations de Solidity:

+-----------------------------------+-------------------------------------------------------+
| Software                          | Notes                                                 |
+===================================+=======================================================+
| `CMake`_ (version 3.13+)          | Cross-platform build file generator.                  |
+-----------------------------------+-------------------------------------------------------+
| `Boost`_ (version 1.77+ on        | C++ libraries.                                        |
| Windows, 1.65+ otherwise)         |                                                       |
+-----------------------------------+-------------------------------------------------------+
| `Git`_                            | Command-line tool for retrieving source code.         |
+-----------------------------------+-------------------------------------------------------+
| `z3`_ (version 4.8+, Optional)    | For use with SMT checker.                             |
+-----------------------------------+-------------------------------------------------------+
| `cvc4`_ (Optional)                | For use with SMT checker.                             |
+-----------------------------------+-------------------------------------------------------+

.. _cvc4: https://cvc4.cs.stanford.edu/web/
.. _Git: https://git-scm.com/download
.. _Boost: https://www.boost.org
.. _CMake: https://cmake.org/download/
.. _z3: https://github.com/Z3Prover/z3

.. note::
    Solidity versions prior to 0.5.10 can fail to correctly link against Boost versions 1.70+.
    A possible workaround is to temporarily rename ``<Boost install path>/lib/cmake/Boost-1.70.0``
    prior to running the cmake command to configure solidity.

    Starting from 0.5.10 linking against Boost 1.70+ should work without manual intervention.

.. note::
    The default build configuration requires a specific Z3 version (the latest one at the time the
    code was last updated). Changes introduced between Z3 releases often result in slightly different
    (but still valid) results being returned. Our SMT tests do not account for these differences and
    will likely fail with a different version than the one they were written for. This does not mean
    that a build using a different version is faulty. If you pass ``-DSTRICT_Z3_VERSION=OFF`` option
    to CMake, you can build with any version that satisfies the requirement given in the table above.
    If you do this, however, please remember to pass the ``--no-smt`` option to ``scripts/tests.sh``
    to skip the SMT tests.

Minimum Compiler Versions
^^^^^^^^^^^^^^^^^^^^^^^^^

The following C++ compilers and their minimum versions can build the Solidity codebase:

- `GCC <https://gcc.gnu.org>`_, version 8+
- `Clang <https://clang.llvm.org/>`_, version 7+
- `MSVC <https://visualstudio.microsoft.com/vs/>`_, version 2019+

Prérequis - macOS
-----------------

Pour macOS, assurez-vous d'avoir installer la dernière version de
`Xcode <https://developer.apple.com/xcode/download/>`_.
Ceci contient le compilateur C++ `Clang <https://en.wikipedia.org/wiki/Clang>`_, l'IDE
`Xcode <https://en.wikipedia.org/wiki/Xcode>`_ et d'autres outils de développement Apple qui sont nécessaires pour construire des applications C++ sous OS X.
Si vous installez Xcode pour la première fois, ou si vous venez d'installer une nouvelle version, vous devrez accepter la licence avant de pouvoir compiler en ligne de commande:

.. code-block:: bash

    sudo xcodebuild -license accept

Nos versions pour OS X exigent que vous installiez le gestionnaire de paquets `Homebrew <http://brew.sh>`_
pour l'installation des dépendances externes.
Voici comment `désinstaller Homebrew
<https://docs.brew.sh/FAQ#how-do-i-uninstall-homebrew>`_,
si vous voulez recommencer à zéro.

Prérequis - Windows
-------------------

Vous aurez besoin des dépendances suivants pour compiler Solidity sous Windows:

+-----------------------------------+-------------------------------------------------------+
| Software                          | Notes                                                 |
+===================================+=======================================================+
| `Visual Studio 2019 Build Tools`_ | C++ compiler                                          |
+-----------------------------------+-------------------------------------------------------+
| `Visual Studio 2019`_  (Optional) | C++ compiler and dev environment.                     |
+-----------------------------------+-------------------------------------------------------+
| `Boost`_ (version 1.77+)          | C++ libraries.                                        |
+-----------------------------------+-------------------------------------------------------+

Si vous avez déjà eu un IDE et que vous n'avez besoin que du compilateur et des bibliothèques,
vous pouvez installer Visual Studio 2019 Build Tools.

Visual Studio 2019 fournit à la fois l'IDE et le compilateur et les bibliothèques nécessaires.
Donc si vous n'avez pas d'IDE et que vous préférez développer en Solidity, Visual Studio 2019
peut être un choix pour tout installer facilement.

Voici la liste des composants à installer
dans Visual Studio 2019 Build Tools ou Visual Studio 2019 :

* Visual Studio C+++ fonctionnalités de base
* VC+++ 2019 v141 toolset (x86,x64)
* Windows Universal CRT SDK
* Windows 8.1 SDK
* Support C+++/CLI

.. _Visual Studio 2019: https://www.visualstudio.com/vs/
.. _Visual Studio 2019 Build Tools: https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2019

We have a helper script which you can use to install all required external dependencies:

.. code-block:: bat

    scripts\install_deps.ps1

This will install ``boost`` and ``cmake`` to the ``deps`` subdirectory.

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

. note::
    This method will result in a prerelease build leading to e.g. a flag
    being set in each bytecode produced by such a compiler.
    If you want to re-build a released Solidity compiler, then
    please use the source tarball on the github release page:

    https://github.com/ethereum/solidity/releases/download/v0.X.Y/solidity_0.X.Y.tar.gz

    (not the "Source code" provided by github).

Compilation en ligne de commande
--------------------------------

**Soyez sûrs d'installer les dépendances externes avant de compiler.**

Le projet Solidity utilise CMake pour la configuration de compilation.
Vous voulez peut-être installer ccache pour accélérer des compilations successives.
CMake l'utilisera automatiquement.
Compiler Solidity est similaire sur Linux, macOS et autres systèmes Unix:

.. _ccache: https://ccache.dev/

.. code-block:: bash

    mkdir build
    cd build
    cmake .. && make

ou même sous Linux et macOS, vous pouvez:

.. code-block:: bash

    #note: les binaires de solc et les tests seront installés dans usr/local/bin
    ./scripts/build.sh

.. warning::

    BSD builds should work, but are untested by the Solidity team.

Et pour Windows:

.. code-block:: bash

    mkdir build
    cd build
    cmake -G "Visual Studio 16 2019" ..

In case you want to use the version of boost installed by ``scripts\install_deps.ps1``, you will
additionally need to pass ``-DBoost_DIR="deps\boost\lib\cmake\Boost-*"`` and ``-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded``
as arguments to the call to ``cmake``.

Ceci devrait aboutir à la création de **solidity.sln** dans ce répertoire de compilation.  Double-cliquer sur ce fichier devrait faire démarrer Visual Studio.  Nous suggérons de construire la configuration **Release**, mais toutes les autres fonctionnent.

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

1. la version 0.4.0 est publiée
2. nightly build a une version de 0.4.1 à partir de maintenant
3. des modifications incessantes sont introduites - pas de changement de version
4. un changement de rupture est introduit - la version est augmentée à 0.5.0
5. la version 0.5.0 est publiée

Ce comportement fonctionne bien avec le :ref:`version pragma <version_pragma>`.
