.. index:: ! installing

.. _installing-solidity:

################################
安装 Solidity 编译器
################################

版本
==========

Solidity 的版本遵循 `语义化版本原则 <https://semver.org>`_。此外，
主版本（例如：0.x.y）的补丁级版本的发布不会包含重大更改。这意味着用 0.x.y 版本
编译的代码可望用 0.x.z 版本编译，其中 z > y。

除了发行版本外，我们还提供 **每日开发构建版本 （nightly development builds）** ，
目的是使开发人员能够轻松地试用即将推出的功能并提供早期反馈。然而，请注意，
虽然每日开发构建版本通常是很稳定的，但它们包含了来自开发分支的前沿代码，
并不保证总是有效的。尽管我们尽了最大努力，
它们仍可能含有未记录的或重大的修改，这些修改不会成为实际发布版本的一部分。
它们也不会用于生产。

当开发智能合约时，您应该使用最新版本的 Solidity。这是因为重大的改变，
以及新的特性和错误修复是定期引入的。
我们目前使用 0.x 版本号 `来表示这种快速的变化的 <https://semver.org/#spec-item-4>`_。

Remix
=====

*我们推荐使用 Remix 来开发简单合约和快速学习 Solidity。*

`Remix 可以在线使用 <https://remix.ethereum.org/>`_，而无需安装任何东西。
如果您想离线使用，可按 https://github.com/ethereum/remix-live/tree/gh-pages
的页面说明下载 ``.zip`` 文件来使用。 Remix 也是一个方便的选择，
可以在不安装多个 Solidity 版本的情况下测试每日开发构建版本。

本页的进一步选项详细说明了在您的计算机上安装 Solidity 命令行编译器。
如果您刚好要处理大型合约，或者需要更多的编译选项，
那么您应该选择使用一个命令行编译器。

.. _solcjs:

npm / Node.js
=============

使用 ``npm`` 可以便捷地安装 ``solcjs`` ，它一个 Solidity 编译器。
但该 `solcjs` 程序的功能比本页下面描述的访问编译器的方法要少。
在 :ref:`commandline-compiler` 一章中，我们假定您使用的是全功能的编译器: ``solc``。
``solcjs`` 的用法在它自己的 `代码仓库 <https://github.com/ethereum/solc-js>`_ 中记录。

注意: `solc-js` 项目是通过使用 Emscripten 从 C++ 版的 `solc` 衍生出来的，
这意味着两者使用相同的编译器源代码。
因此， `solc-js` 可以直接用于JavaScript项目（如 Remix） 具体介绍请参考 `solc-js` 代码库。


.. code-block:: bash

    npm install -g solc

.. note::

    在命令行中，可执行文件被命名为 ``solcjs``。

    ``solcjs`` 的命令行选项与 ``solc`` 和一些工具（如 ``geth``）是不兼容的，
    因此不要期望 ``solcjs`` 能像 ``solc`` 一样工作。

Docker
======

Solidity构建的Docker镜像可以使用从 ``ethereum`` 组织获得的 ``solc`` 镜像。
使用 ``stable`` 标签获取最新发布的版本，使用 ``nightly`` 标签获取开发分支中潜在的不稳定变更的版本。

Docker镜像会运行编译器可执行文件，所以您可以把所有的编译器参数传给它。
例如，下面的命令提取了稳定版的 ``solc`` 镜像（如果您还没有），
并在一个新的容器中运行它，同时传递 ``--help`` 参数。

.. code-block:: bash

    docker run ethereum/solc:stable --help

您也可以在标签中指定发行的版本，例如，0.5.4版本。

.. code-block:: bash

    docker run ethereum/solc:0.5.4 --help

要使用 Docker 镜像来编译主机上的 Solidity 文件，请安装一个本地文件夹
用于输入和输出，并指定要编译的合约。例如：

.. code-block:: bash

    docker run -v /local/path:/sources ethereum/solc:stable -o /sources/output --abi --bin /sources/Contract.sol

您也可以使用标准的JSON接口（当使用工具化的编译器时建议使用这种方式）。
当使用这个接口时，不需要装载任何目录，只要输入的 JSON 是自成一体的
（即它没有引用任何外部文件，而这些文件必须要被
:ref:`由导入回调 <initial-vfs-content-standard-json-with-import-callback>`)。

.. code-block:: bash

    docker run ethereum/solc:stable --standard-json < input.json > output.json

Linux 包
==============

Solidity 的二进制安装包可在 `solidity/releases <https://github.com/ethereum/solidity/releases>`_ 找到。

对于 Ubuntu ，我们也提供 PPAs 。通过以下命令，可获取最新的稳定版本：

.. code-block:: bash

    sudo add-apt-repository ppa:ethereum/ethereum
    sudo apt-get update
    sudo apt-get install solc

您也可以使用以下命令安装每日开发构建版本：

.. code-block:: bash

    sudo add-apt-repository ppa:ethereum/ethereum
    sudo add-apt-repository ppa:ethereum/ethereum-dev
    sudo apt-get update
    sudo apt-get install solc

此外，一些 Linux 发行版提供了他们自己的软件包。这些软件包不是由我们直接维护的，
而通常由各自的软件包维护者保持最新。

例如，Arch Linux 也有最新开发版本的软件包。

.. code-block:: bash

    pacman -S solidity

还有一个 `snap包 <https://snapcraft.io/solc>`_，然而，它 **目前没有维护** 。
它可以安装在所有 `支持的Linux发行版 <https://snapcraft.io/docs/core/install>`_ 。通过以下命令，
安装最新的稳定版本的 solc：

.. code-block:: bash

    sudo snap install solc

如果您想测试 develop 分支下的最新变更，请使用以下方式：

.. code-block:: bash

    sudo snap install solc --edge

.. note::

    ``solc`` snap 使用严格的限制。这对 snap 包来说是最安全的模式
    但它也有一些限制，比如只能访问 ``/home`` 和 ``/media`` 目录下的文件。
    欲了解更多信息，请访问 `Demystifying Snap Confinement <https://snapcraft.io/blog/demystifying-snap-confinement>`_。

macOS Packages
==============

我们通过 Homebrew 作为从源头建立的版本, 发布 Solidity 编译器，。目前不支持预构建。

.. code-block:: bash

    brew update
    brew upgrade
    brew tap ethereum/ethereum
    brew install solidity

要安装最新的 0.4.x/0.5.x 版本的 Solidity，您也可以分别使用 ``brew install solidity@4``
和 ``brew install solidity@5``。

如果您需要特定版本的 Solidity，您可以直接从 Github 上安装一个 Homebrew 列表。

参见
`solidity.rb 在 Github 上的提交情况 <https://github.com/ethereum/homebrew-ethereum/commits/master/solidity.rb>`_.

复制您想要的版本的提交哈希值，然后在您的机器上检出该分支。

.. code-block:: bash

    git clone https://github.com/ethereum/homebrew-ethereum.git
    cd homebrew-ethereum
    git checkout <your-hash-goes-here>

使用 ``brew`` 安装:

.. code-block:: bash

    brew unlink solidity
    # 例如，安装 0.4.8
    brew install solidity.rb

静态二进制文件
===============

我们在 `solc-bin`_ 上维护了一个包含过去和现在编译器版本的静态构建的资源库，用于所有支持的平台。
您也可以找到每日开发构建版本。

该资源库不仅是一个快速且简单的方法，让终端用户获得可以开箱即用的二进制文件，
而且它对第三方工具也很友好：

- 这些内容被镜像到 https://binaries.soliditylang.org，在那里可以很容易地通过 HTTPS 下载，
  没有任何认证、速率或需要使用git的限制。
- 提供的内容具有正确的 `Content-Type` 请求头和宽松的 CORS 配置，
  因此它可以被运行在浏览器中的工具直接加载。
- 二进制文件不需要安装或解包（与必要的 DLLs 捆绑在一起的旧版 Windows 除外）。
- 我们努力争取高水平的向后兼容性。文件一旦被添加，在没有提供旧位置的链接/重定向的情况下，不会被删除或移动。
  它们也不会被修改，而且应始终与原始校验相匹配。唯一的例外是破损或无法使用的文件，
  如果保持原样，有可能造成更大的伤害。
- 文件是通过 HTTP 和 HTTPS 提供的。只要您以安全的方式获得文件列表
  （通过 git、HTTPS、IPFS 或者只是在本地的缓存），并在下载后验证二进制文件的哈希值，
  您就不必通过HTTPS获得二进制文件。

在大多数情况下，同样的二进制文件可以在 `Github 上的 Solidity 发布页 <https://github.com/ethereum/solidity/releases>`_ 中找到。
不同的是，我们一般不更新Github已发布的旧版本。这意味着如果命名规则改变，我们不会重新命名，
也不会为发布时不支持的平台添加构建。这只发生在 ``solc-bin`` 资源库里。

``solc-bin`` 资源库包含几个顶级目录，每个目录代表一个平台。
每个目录都包含一个 ``list.json`` 文件，列出可用的二进制文件。
例如，在 ``emscripten-wasm32/list.json`` 中您会发现以下关于 0.7.4 版本的信息。

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

这意味着：

- 您可以在同一目录下找到二进制文件，名称为
  `solc-emscripten-wasm32-v0.7.4+commit.3f05b770.js <https://github.com/ethereum/solc-bin/blob/gh-pages/emscripten-wasm32/solc-emscripten-wasm32-v0.7.4+commit.3f05b770.js>`_.
  注意，该文件可能是一个软链接，如果您没有使用 git 下载，或者您的文件系统不支持软链接，您需要自己解决。
- 该二进制文件也被镜像在 https://binaries.soliditylang.org/emscripten-wasm32/solc-emscripten-wasm32-v0.7.4+commit.3f05b770.js.
  在这种情况下，不需要 git，软链接的解决方式是显而易见的，要么提供一个文件的副本，要么返回一个 HTTP 重定向。
- 该文件也可在 IPFS上 找到，地址是 `QmTLs5MuLEWXQkths41HiACoXDiH8zxyqBHGFDRSzVE5CS`_.
- 该文件将来可能会存储在 Swarm 上，
  地址是 `16c5f09109c793db99fe35f037c6092b061bd39260ee7a677c8a97f18c955ab1`_.
- 您可以通过比较其keccak256哈希值来验证二进制文件的完整性
  ``0x300330ecd127756b824aa13e843cb1f43c473cb22eaf3750d5fb9c99279af8c3``。哈希值可以在命令行上
  使用 `sha3sum`_ 提供的 ``keccak256sum`` 工具
  或在 JavaScript 中使用 `ethereumjs-util 的 keccak256() 函数。`
- 您也可以通过比较二进制文件的sha256哈希值来验证它的完整性
  ``0x2b55ed5fec4d9625b6c7b3ab1abd2b7fb7dd2a9c68543bf0323db2c7e2d55af2``。

.. warning::

   由于高度的向后兼容性要求，版本库包含一些遗留元素，但您在编写新工具时应避免使用它们：

   - 如果您想获得最佳的性能，请使用 ``emscripten-wasm32/`` （有回退功能的 ``emscripten-asmjs/``）而不是 ``bin/``。
     在 0.6.1 版本之前，我们只提供 asm.js 二进制文件。从 0.6.2 开始，我们改用 `WebAssembly builds`_，性能好得多。
     我们已经为wasm重建了旧版本，但原来的asm.js文件仍然在 ``bin/`` 下。
     新的文件必须放在一个单独的目录中，以避免名称冲突。
   - 如果您想确定下载的是 wasm 还是 asm.js 二进制文件，请使用 ``emscripten-asmjs/`` 和 ``emscripten-wasm32/``
     而不是 ``bin/`` 和 ``wasm/`` 目录。
   - 使用 ``list.json`` 代替 ``list.js`` 和 ``list.txt``。JSON列表格式包含了旧列表的所有信息。
   - 使用 https://binaries.soliditylang.org，而不是 https://solc-bin.ethereum.org。
     为了使事情简单化，我们把几乎所有与编译器有关的东西都移到了新的域名 ``soliditylang.org`` 下，
     这也适用于 ``solc-bin``。虽然推荐使用新的域名，但旧的域名仍然被完全支持，并保证指向同一位置。

.. warning::

    二进制文件也可以在 https://ethereum.github.io/solc-bin/ 找到，
    但这个页面在 0.7.2 版本发布后就停止了更新，不会收到任何平台的新版本或每日开发构建版本，
    也不提供新的目录结构，包括非 emscripten 的构建。

    如果您正在使用它，请切换到 https://binaries.soliditylang.org，它是一个直接的替代。
    这使我们能够以透明的方式对底层主机进行更改，并尽量减少干扰。
    与我们无法控制的 ``ethereum.github.io`` 域名不同，
    ``binaries.soliditylang.org`` 可以保证长期运行并保持相同的URL结构。

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

从源代码编译
====================

先决条件 - 所有操作系统
-------------------------------------

以下是 Solidity 构建的所有依赖性：

+-------------------------------+------------------------------+
|             软件              |             备注             |
+===============================+==============================+
| `CMake`_ (3.13以上版本)       | 跨平台构建文件生成器。       |
+-------------------------------+------------------------------+
| `Boost`_ (Windows系统         | C++ 库。                     |
| 3.13以上版本, 其他系统1.65+ ) |                              |
+-------------------------------+------------------------------+
| `Git`_                        | 用于获取源代码的命令行工具。 |
+-------------------------------+------------------------------+
| `z3`_ (4.8以上版本, 可选)     | 与SMT检查器一起使用。        |
+-------------------------------+------------------------------+
| `cvc4`_ (可选)                | 与SMT检查器一起使用。        |
+-------------------------------+------------------------------+

.. _cvc4: https://cvc4.cs.stanford.edu/web/
.. _Git: https://git-scm.com/download
.. _Boost: https://www.boost.org
.. _CMake: https://cmake.org/download/
.. _z3: https://github.com/Z3Prover/z3

.. note::
    0.5.10 之前的 Solidity 版本可能无法与 Boost 1.70 以上版本正确链接。
    一个可能的解决方法是，在运行 cmake 命令配置 Solidity 之前，暂时重命名 ``<Boost install path>/lib/cmake/Boost-1.70.0``。

    从 0.5.10 开始，针对 Boost 1.70 以上版本的链接应该无需人工干预。

.. note::
    默认的构建配置需要一个特定的 Z3 版本（在代码最后更新时的最新版本）。
    Z3 版本之间的变化常常导致返回的结果略有不同（但仍然有效）。
    我们的SMT测试没有考虑到这些差异，很可能会在不同的版本中失败，而不是为其编写的版本。
    这并不意味着使用不同版本的构建是有问题的。如果将 ``-DSTRICT_Z3_VERSION=OFF`` 选项传递给CMake，
    您可以使用任何满足上表要求的版本进行构建。
    然而，如果您这样做，请记得在 ``scripts/tests.sh`` 中传递 ``--no-smt`` 选项以跳过SMT测试。

最小编译器版本
^^^^^^^^^^^^^^^^^^^^^^^^^

以下C++编译器及其最小版本可构建 Solidity 代码库：

- `GCC <https://gcc.gnu.org>`_, 8以上版本
- `Clang <https://clang.llvm.org/>`_, 7以上版本
- `MSVC <https://visualstudio.microsoft.com/vs/>`_, 2019以上版本

先决条件 - macOS
---------------------

对于 macOS 的构建，确保最新版本的 `Xcode 已安装 <https://developer.apple.com/xcode/download/>`_。
这包含了 `Clang C++ 编译器 <https://en.wikipedia.org/wiki/Clang>`_，
`Xcode IDE <https://en.wikipedia.org/wiki/Clang>`_ 和其他苹果公司的开发工具，
这些工具是在 OS X 上构建 C++ 应用程序所必须的。
如果您是第一次安装 Xcode，或者刚刚安装了一个新的版本，那么您在使用命令行构建前，需同意使用协议：

.. code-block:: bash

    sudo xcodebuild -license accept

我们的 OS X 构建脚本使用 `the Homebrew <https://brew.sh>`_
软件包管理器来安装外部依赖。
如果您想从头开始的话，以下是如何 `卸载Homebrew
<https://docs.brew.sh/FAQ#how-do-i-uninstall-homebrew>`_。


先决条件 - Windows
-----------------------

您需要为 Solidity 的 Windows 版本安装以下依赖软件包:

+-----------------------------------+------------------------+
|             Software              |         Notes          |
+===================================+========================+
| `Visual Studio 2019 Build Tools`_ | C++ 编译器。           |
+-----------------------------------+------------------------+
| `Visual Studio 2019`_  (可选)     | C++ 编译器和开发环境。 |
+-----------------------------------+------------------------+
| `Boost`_ (1.77版本以上)           | C++ 库文件。           |
+-----------------------------------+------------------------+

如果您已经有一个 IDE 并且只需要编译器和库文件。您可以安装 Visual Studio 2019 构建工具。

Visual Studio 2019 同时提供IDE和必要的编译器和库。
所以，如果您没有一个 IDE，并且想要开发 Solidity，
那么 Visual Studio 2019 将是一个可以使您轻松获得一切设置的选择。

以下是应在 Visual Studio 2019 构建工具或 Visual Studio 2019 中安装的组件列表：

* Visual Studio C++ core features
* VC++ 2019 v141 toolset (x86,x64)
* Windows Universal CRT SDK
* Windows 8.1 SDK
* C++/CLI support

.. _Visual Studio 2019: https://www.visualstudio.com/vs/
.. _Visual Studio 2019 Build Tools: https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2019

我们有一个辅助脚本，您可以用它来安装所有需要的外部依赖：

.. code-block:: bat

    scripts\install_deps.ps1

这将安装 ``boost`` 和 ``cmake`` 到 ``deps`` 子目录。

克隆代码库
--------------------

执行以下命令，克隆源代码：

.. code-block:: bash

    git clone --recursive https://github.com/ethereum/solidity.git
    cd solidity

如果您想帮助开发 Solidity，
您可以分叉 Solidity，然后将您个人的分叉库作为第二远程源添加。

.. code-block:: bash

    git remote add personal git@github.com:[username]/solidity.git

.. note::
    这种方法将导致一个预发布的构建，例如，在这种编译器产生的每个字节码中设置一个标志。
    如果您想重新构建一个已发布的 Solidity 编译器，那么请使用 github 发布页上的源压缩包：

    https://github.com/ethereum/solidity/releases/download/v0.X.Y/solidity_0.X.Y.tar.gz

    (而不是由 github 提供的 "源代码")。

命令行构建
------------------

**请确保在构建前安装外部依赖项（见上文）。**

Solidity 项目使用 CMake 来配置构建。
您可能想安装 `ccache`_ 以加快重复构建的速度。CMake 会自动使用它。
在 Linux、macOS 和其他 Unix 系统上构建 Solidity 方式都差不多：

.. _ccache: https://ccache.dev/

.. code-block:: bash

    mkdir build
    cd build
    cmake .. && make

或者在 Linux 和 macOS 上有更简单的方式，您可以运行：

.. code-block:: bash

    #注意：这将在 usr/local/bin 安装 solc 和 soltest 的二进制文件。
    ./scripts/build.sh

.. warning::

    BSD 构建应该也可以工作，但是 Solidity 团队没有测试过。

对于 Windows 执行：

.. code-block:: bash

    mkdir build
    cd build
    cmake -G "Visual Studio 16 2019" ..

如果您想使用由 ``scripts\install_deps.ps1`` 安装的 boost 版本，
您需要额外传递 ``-DBoost_DIR="deps\boost\lib\cmake\Boost-*"`` 和 ``-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded``
作为参数给 ``cmake`` 调用。

这将会导致在构建目录中创建 **solidity.sln** 文件。
双击该文件，Visual Studio 就会启动。
我们建议创建 **Release** 配置，但其他的配置也可以。

或者，您可以在命令行上为 Windows 构建，像这样：

.. code-block:: bash

    cmake --build . --config Release

CMake 选项
=============

如果您对CMake的可选项感兴趣，可以运行 ``cmake ... -LH``。

.. _smt_solvers_build:

SMT 解算器
-----------
Solidity 可以针对 SMT 解算器进行构建，如果它们在系统中被发现，
将默认为是这样做的。每个解算器都可以通过 `cmake` 选项禁用。

*注意：在某些情况下，这也可以是构建失败后，可能的变通方法。*


在构建文件夹内，您可以禁用它们，因为它们是默认启用的:

.. code-block:: bash

    # 只禁用Z3 SMT解算器。
    cmake .. -DUSE_Z3=OFF

    # 只禁用CVC4 SMT解算器。
    cmake .. -DUSE_CVC4=OFF

    # 同时禁用Z3和CVC4
    cmake .. -DUSE_CVC4=OFF -DUSE_Z3=OFF

版本号字符串详解
============================

Solidity 版本名包含四部分：

- 版本号
- 预发布版本标签，通常为 ``develop.YYYY.MM.DD`` 或者 ``nightly.YYYY.MM.DD``
- 以 ``commit.GITHASH`` 格式展示的提交号
- 由若干条平台、编译器详细信息构成的平台标识

如果有本地修改，提交将会有后缀 ``.mod``。

这些部分按照 Semver 的要求来组合， 其中 Solidity 预发布版标签等价于 Semver 预发布版标签，
而 Solidity 提交号和平台标识则组成Semver的构建元数据。

发布版样例: ``0.4.8+commit.60cc1668.Emscripten.clang``。

预发布版样例: ``0.4.9-nightly.2017.1.17+commit.6ecb4aa3.Emscripten.clang``。

关于版本管理的重要信息
======================================

在版本发布之后，补丁版本号会增加，因为我们假定接下来只有补丁级别的变更。
当变更被合并后，版本应该根据 Semver 和变更的重要程度来提升。
最后，发行版本总是与当前每日开发构建版本本的版本号一致，但没有 ``prerelease`` 指示符。


示例:

1. 0.4.0 版本发布。
2. 从现在开始，每晚构建一个 0.4.1 版本。
3. 引入非重大变更 —— 不改变版本号。
4. 引入重大变更 —— 版本号提升到 0.5.0。
5. 0.5.0 版本发布。

该方式与 :ref:`version pragma <version_pragma>` 一起运行良好。
