.. _path-resolution:

**********************
导入路径解析
**********************

为了能够在所有平台上支持可重复的构建，Solidity 编译器必须抽象出存储源文件的文件系统的细节。
在导入中使用的路径必须在任何地方以同样的方式工作，而命令行界面必须能够与平台特定的路径一起工作，
以提供良好的用户体验。
本节旨在详细解释 Solidity 是如何协调这些要求的。

.. index:: ! virtual filesystem, ! VFS, ! source unit name
.. _virtual-filesystem:

虚拟文件系统
==================

编译器维护一个内部数据库（ *虚拟文件系统* 或简称 *VFS* ），
每个源单元被分配一个唯一的 *源单元名称*，这是一个不透明的非结构化的标识符。
当您使用 :ref:`import 语句 <import>` 时，
您指定了引用源单元名称的 *导入路径*。

.. index:: ! import callback, ! Host Filesystem Loader
.. _import-callback:

导入回调
---------------

VFS最初只填充了编译器收到的输入文件。
在编译过程中可以使用 *import 回调* 加载其他文件，
但这取决于您使用的编译器的类型（见下文）。
如果编译器在VFS中没有找到任何与导入路径相匹配的源单元名称，
它就会调用回调，负责获取要放在该名称下的源代码。
一个导入回调可以自由地以任意方式解释源单元名称，而不仅仅是作为路径。
如果在需要回调时没有可用的回调，或者无法找到源代码，编译就会失败。

命令行编译器提供了 *主机文件系统加载器* -- 一个基本的回调，
它将源单元名称解释为本地文件系统中的一个路径。
`JavaScript接口 <https://github.com/ethereum/solc-js>`_ 默认不提供任何接口，
但可以由用户提供一个。
这个机制可以用来从本地文件系统以外的地方获得源代码
（本地文件系统甚至可能无法访问，例如当编译器在浏览器中运行时）。
例如， `Remix IDE <https://remix.ethereum.org/>`_ 提供了一个多功能的回调，
让您 `从HTTP、IPFS和Swarm URL导入文件，或直接引用NPM注册表中的包 <https://remix-ide.readthedocs.io/en/latest/import.html>`_。

.. note::

    主机文件系统加载器的文件查找是依赖于平台的。
    例如，源单元名称中的反斜线可以被解释为目录分隔符，也可以不被解释为目录分隔符，
    查找时可以区分大小写，这取决于底层平台。

    为了实现可移植性，我们建议避免使用只有在特定的导入回调中才能正常工作的导入路径，
    或者只在一个平台上使用。
    例如，您应该总是使用正斜线，因为它们在支持反斜线的平台上也能作为路径分隔符使用。

虚拟文件系统的初始内容
-----------------------------------------

VFS的初始内容取决于您如何调用编译器：

#. **solc / 命令行界面**

   当您使用编译器的命令行界面编译一个文件时，您提供一个或多个包含Solidity代码的文件的路径：

   .. code-block:: bash

       solc contract.sol /usr/local/dapp-bin/token.sol

   以这种方式加载的文件的源单元名称是通过将其路径转换为规范的形式来构建的，
   如果可能的话，使其与基本路径或其中一个包含路径相对。
   参见 :ref:`CLI路径规范化和剥离 <cli-path-normalization-and-stripping>` 以了解这一过程的详细描述。

   .. index:: standard JSON

#. **标准JSON**

   当使用 :ref:`标准JSON <compiler-api>` API时
   通过 `JavaScript接口 <https://github.com/ethereum/solc-js>`_ 或
   `--standard-json` 命令行选项），您需提供JSON格式的输入，其中包含您所有源文件的内容。

   .. code-block:: json

       {
           "language": "Solidity",
           "sources": {
               "contract.sol": {
                   "content": "import \"./util.sol\";\ncontract C {}"
               },
               "util.sol": {
                   "content": "library Util {}"
               },
               "/usr/local/dapp-bin/token.sol": {
                   "content": "contract Token {}"
               }
           },
           "settings": {"outputSelection": {"*": { "*": ["metadata", "evm.bytecode"]}}}
       }

   上面的 ``sources`` 字典结构成为虚拟文件系统的初始内容，它的键被用作源单元名称。

   .. _initial-vfs-content-standard-json-with-import-callback:

#. **标准JSON（通过导入回调）**

   通过标准JSON，也可以告诉编译器使用导入回调来获得源代码：

   .. code-block:: json

       {
           "language": "Solidity",
           "sources": {
               "/usr/local/dapp-bin/token.sol": {
                   "urls": [
                       "/projects/mytoken.sol",
                       "https://example.com/projects/mytoken.sol"
                   ]
               }
           },
           "settings": {"outputSelection": {"*": { "*": ["metadata", "evm.bytecode"]}}}
       }

   如果导入回调是可用的，编译器将一个一个地给它 ``urls`` 中指定的字符串，直到有一个被成功加载或到达列表的末尾。

   源单元名称的确定方式与使用 ``content`` 时相同 - 它们是 ``sources`` 字典结构的键，
   ``urls`` 的内容不会以任何方式影响它们。

   .. index:: standard input, stdin, <stdin>

#. **标准输入**

   在命令行中，也可以通过将源代码发送到编译器的标准输入来提供源代码:

   .. code-block:: bash

       echo 'import "./util.sol"; contract C {}' | solc -

   ``-`` 作为参数之一，指示编译器将标准输入的内容放在虚拟文件系统中的一个特殊的源单元名下： ``<stdin>``。

初始化VFS之后，仍然可以向它添加其他文件，但只能通过导入回调的方式。

.. index:: ! import; path

导入
=======

导入语句指定了一个 *导入路径*。
根据导入路径的指定方式，我们可以将导入分为两类：

- :ref:`直接导入 <direct-imports>`，直接指定完整的源单元名称。
- :ref:`相对导入 <relative-imports>`，指定一个以 ``./`` 或 ``../`` 开头的路径，
  与导入文件的源单元名称相结合。


.. code-block:: solidity
    :caption: contracts/contract.sol

    import "./math/math.sol";
    import "contracts/tokens/token.sol";

在上面的 ``./math/math.sol`` 和 ``contracts/tokens/token.sol`` 都是导入路径，
然而它们转译成的源单元名分别是 ``contracts/math/math.sol`` 和 ``contracts/tokens/token.sol``。

.. index:: ! direct import, import; direct
.. _direct-imports:

直接导入
--------------

不以 ``./`` 或 ``../`` 开头的导入是 *直接导入*。

.. code-block:: solidity

    import "/project/lib/util.sol";         // 源单元名称： /project/lib/util.sol
    import "lib/util.sol";                  // 源单元名称： lib/util.sol
    import "@openzeppelin/address.sol";     // 源单元名称： @openzeppelin/address.sol
    import "https://example.com/token.sol"; // 源单元名称： https://example.com/token.sol

在应用任何 :ref:`导入重映射 <import-remapping>` 之后，导入路径简单地成为源单元名称。

.. note::

    一个源单元的名字只是一个标识符，即使它的值碰巧看起来像一个路径，
    它也不受您在shell中通常期望的规范化规则的约束。
    任何 ``/./`` 或 ``/../`` 的分隔符或多个斜线的序列都是它的一部分。
    当源是通过标准JSON接口提供的时候，完全有可能将不同的内容与源单元的名称联系起来，
    这些名称将指代磁盘上的同一个文件。

当源文件在虚拟文件系统中不可用时，编译器会将源单元名称传递给导入回调。
主机文件系统加载器将尝试使用它作为路径并在磁盘上查找文件。
在这一点上，平台特定的规范化规则开始发挥作用，在VFS中被认为是不同的名字实际上可能导致同一个文件被加载。
例如， ``/project/lib/math.sol`` 和 ``/project/lib/../lib///math.sol``
在VFS中被认为是完全不同的，但它们在磁盘上指向的是同一个文件。

.. note::

    即使一个导入回调最终从磁盘上的同一个文件中加载了两个不同的源单元名称的源代码，
    编译器仍然会将它们视为独立的源单元。
    重要的是源单元名称，而不是代码的物理位置。

.. index:: ! relative import, ! import; relative
.. _relative-imports:

相对导入
----------------

以 ``./`` 或 ``./`` 开头的导入是一个 *相对导入*。
这种导入指定了一个相对于导入源单元的源单元名称的路径。

.. code-block:: solidity
    :caption: /project/lib/math.sol

    import "./util.sol" as util;    // 源单元名称： /project/lib/util.sol
    import "../token.sol" as token; // 源单元名称： /project/token.sol

.. code-block:: solidity
    :caption: lib/math.sol

    import "./util.sol" as util;    // 源单元名称： lib/util.sol
    import "../token.sol" as token; // 源单元名称： token.sol

.. note::

    相对导入 **总是** 以 ``./`` 或 ``./`` 开始，
    所以与 ``import "./util.sol"`` 不同， ``import "util.sol"`` 是一个直接导入。
    虽然这两个路径在主机文件系统中被认为是相对的，但 ``util.sol`` 在VFS中实际上是绝对的。

让我们把 *路径段* 定义为路径中不包含分隔符的任何非空部分，并以两个路径分隔符为界。
分隔符是一个正斜杠或字符串的开头/结尾。
例如，在 ``./abc/...//`` 中，有三个路径段。 ``.``, ``abc`` 和 ``..``。

编译器以下列方式从导入路径中计算出一个源单元的名称：

1. 首先计算出一个前缀

   - 前缀被初始化为导入源单元的源单元名称。
   - 最后一个带有斜线的路径段被从前缀中删除。
   - 然后，考虑规范化导入路径的前导部分，仅由 ``/`` 和 ``.`` 字符组成。
     对于在这部分发现的每一个 ``..`` 段，最后一个带斜杠的路径段将从前缀中删除。

2. 然后，前缀被预置到规范化的导入路径中。
   如果前缀不为空，则在它和导入路径之间插入一个单斜线。

删除前面有斜线的最后一个路径段，可以理解为工作原理如下：

1. 超过最后一个斜线的所有内容都被删除（即 ``a/b//c.sol`` 变成 ``a/b//``）。
2. 所有的尾部斜线被删除（即 ``a/b//`` 变成 ``a/b``）。

归一化规则与UNIX路径相同，即：

- 所有内部的 ``.`` 段被删除。
- 每一个内部的 ``..`` 段都在层次结构中往上追溯一级。
- 多条斜线被压成一条。

注意，规范化只在导入路径上执行。
用于前缀的导入模块的源单元名称仍未被规范化。
这确保了在导入文件被识别为URL时， ``protocol://`` 部分不会变成 ``protocol:/``。

如果导入路径已经规范化，则可以期望上述算法产生非常直观的结果。
下面是一些例子，告诉您如果不是的话会发生什么：

.. code-block:: solidity
    :caption: lib/src/../contract.sol

    import "./util/./util.sol";         // 源单元名称： lib/src/../util/util.sol
    import "./util//util.sol";          // 源单元名称： lib/src/../util/util.sol
    import "../util/../array/util.sol"; // 源单元名称： lib/src/array/util.sol
    import "../.././../util.sol";       // 源单元名称： util.sol
    import "../../.././../util.sol";    // 源单元名称： util.sol

.. note::

    不建议使用使用包含前缀 ``..`` 的路径段。
    通过使用带有 :ref:`基本路径和包含路径 <base-and-include-paths>` 的直接导入，
    可以以更可靠的方式实现同样的效果。

.. index:: ! base path, ! --base-path, ! include paths, ! --include-path
.. _base-and-include-paths:

基本路径和包含路径
===========================

基本路径和包含路径表示主机文件系统加载器将加载文件的目录。
当一个源单元的名字被传递给加载器时，它把基本路径加到它的前面，并执行一个文件系统查找。
如果查找不成功，也会对包含路径列表中的所有目录进行同样的处理。

建议将基本路径设置为您项目的根目录，并使用包含路径来指定可能包含您项目所依赖的库的其他位置。
这可以让您以统一的方式从这些库中导入，无论它们在文件系统中相对于您的项目位于何处。
例如，如果您使用npm安装包，而您的合约导入了 ``@openzeppelin/contracts/utils/Strings.sol``，
您可以使用这些选项来告诉编译器，该库可以在npm包目录中找到。

.. code-block:: bash

    solc contract.sol \
        --base-path . \
        --include-path node_modules/ \
        --include-path /usr/local/lib/node_modules/

无论您是把库安装在本地还是全局包目录下，甚至直接安装在您的项目根目录下，
您的合约都会被编译（具有完全相同的元数据）。

默认情况下，基本路径是空的，这使得源单元的名称没有变化。
当源单元名称是一个相对路径时，这将导致文件在编译器被调用的目录中被查找。
这也是唯一能使源单元名称中的绝对路径被实际解释为磁盘上的绝对路径的值。
如果基本路径本身是相对的，则它被解释为相对于编译器的当前工作目录。

.. note::

    包含路径不能有空值，必须与非空的基本路径一起使用。

.. note::

    只要不使导入解析产生歧义，包含路径和基本路径可以重合。
    例如，您可以在基本路径内指定一个目录作为包含目录，或者有一个包含目录是另一个包含目录的子目录。
    只有传递给主机文件系统加载器的源单元名称在与多个包含路径或包含路径和基本路径结合代表一个现有路径时，
    编译器才会发出错误。

.. _cli-path-normalization-and-stripping:

CLI路径规范化和剥离
------------------------------------

在命令行中，编译器的行为就像您对其他程序的期望一样：
它接受平台的本地格式的路径，相对路径是相对于当前工作目录的。
然而，分配给在命令行上指定了路径的文件的源单元名称，不应该因为项目在不同的平台上被编译，
或者因为编译器碰巧从不同的目录被调用而改变。
为了达到这个目的，来自命令行的源文件的路径必须被转换为规范的形式，
如果可能的话，应使其与基本路径或包含路径之一相对。

规范化规则如下：

- 如果一个路径是相对路径，则通过在其前面加上当前工作目录使其成为绝对路径。
- 内部的 ``.`` 和 ``..`` 段被折叠起来。
- 平台特定的路径分隔符被替换为正斜杠。
- 多个连续路径分隔符的序列被压缩成一个分隔符
  （除非它们是 `UNC路径 <https://en.wikipedia.org/wiki/Path_(computing)#UNC>`_ 的前导斜杠）。
- 如果路径中包含一个根名（例如Windows系统中的一个盘符），并且该根名与当前工作目录的根名相同，
  则根名将被替换为 ``/``。
- 路径中的符号链接 **没有** 解析。

  - 唯一的例外是在使相对路径成为绝对路径的过程中，对当前工作目录的路径进行了预处理。
    在一些平台上，工作目录总是用带有符号链接的解析来声明，
    所以为了保持一致性，编译器在任何地方都会解析它们。

- 即使文件系统对大小写不敏感，
  但 `保留大小写 <https://en.wikipedia.org/wiki/Case_preservation>`_
  和磁盘上的实际大小写不同，是会保留路径的原始大小写。

.. note::

    有些情况下，路径不能独立于平台。
    例如，在Windows下，编译器可以通过将当前驱动器的根目录称为 ``/`` 来避免使用驱动器字母，
    但对于通往其他驱动器的路径来说，驱动器字母仍然是必要的。
    您可以通过确保所有的文件都在同一驱动器上的单一目录树内，来避免这种情况。

在规范化之后，编译器试图使源文件的路径变成相对的。
它首先尝试基本路径，然后按照给出的顺序尝试包含路径。
如果基本路径是空的或者没有指定，它将被视为等同于当前工作目录的路径（解决了所有符号链接）。
只有当规范化的目录路径是规范化的文件路径的确切前缀时，才会接受这个结果。
否则，文件路径仍然是绝对的。
这使得转换毫不含糊，并确保相对路径不以 ``.../`` 开头。
产生的文件路径成为源单元名称。

.. note::

    剥离后产生的相对路径必须在基本路径和包含路径中保持唯一。
    例如，如果 ``/project/contract.sol`` 和  ``/lib/contract.sol`` 同时存在，
    编译器将对以下命令发出错误：

    .. code-block:: bash

        solc /project/contract.sol --base-path /project --include-path /lib

.. note::

    在0.8.8版本之前，CLI路径剥离不被执行，唯一应用的规范化是路径分隔符的转换。
    当使用旧版本的编译器时，建议从基本路径调用编译器，在命令行上只使用相对路径。

.. index:: ! allowed paths, ! --allow-paths, remapping; target
.. _allowed-paths:

允许的路径
=============

作为一项安全措施，主机文件系统加载器将拒绝从默认认为安全的几个位置之外的地方加载文件：

- 标准JSON模式之外：

  - 含有命令行上所列输入文件的目录。
  - 作为 :ref:`重映射 <import-remapping>` 目标使用的目录。
    如果目标不是一个目录（即不以 ``/``， ``/.`` 或 ``/.`` 结尾），则使用包含该目标的目录。
  - 基本路径和包含路径。

- 在标准JSON模式下：

  - 基本路径和包含路径。

可以使用 ``--allow-paths`` 选项将其他目录列入白名单。
该选项接受一个用逗号分隔的路径列表：

.. code-block:: bash

    cd /home/user/project/
    solc token/contract.sol \
        lib/util.sol=libs/util.sol \
        --base-path=token/ \
        --include-path=/lib/ \
        --allow-paths=../utils/,/tmp/libraries

当用上面的命令调用编译器时，主机文件系统加载器将允许从以下目录导入文件：

- ``/home/user/project/token/`` （因为 ``token/`` 包含输入文件，也因为它是基本路径），
- ``/lib/`` （因为 ``/lib/`` 是包含路径之一），
- ``/home/user/project/libs/`` （因为 ``libs/`` 是一个包含重映射目标的目录），
- ``/home/user/utils/`` （因为 ``.../utils/`` 传给了 ``-allow-paths`` ），
- ``/tmp/libraries/`` （因为 ``/tmp/libraries`` 被传递到 ``/tmp/libraries``），

.. note::

    编译器的工作目录是默认允许的路径之一，前提是它恰好是基本路径时（或者基本路径没有被指定或有一个空值）。

.. note::

    编译器不检查允许的路径是否真实存在以及它们是否是目录。
    不存在的或空的路径会被简单地忽略掉。
    如果一个被允许的路径与一个文件而不是一个目录相匹配，该文件也被视为白名单。

.. note::

    允许的路径是区分大小写的，即使文件系统不是这样的。
    大小写必须与您的导入中使用的大小写完全一致。
    例如 ``--allow-paths tokens`` 不会匹配 ``import "Tokens/IERC20.sol"``。

.. warning::

    只有通过允许的目录的符号链接才能到达的文件和目录不会被自动列入白名单。
    例如，如果上面的例子中的 ``token/contract.sol`` 实际上是一个指向
    ``/etc/passwd`` 的符号链接，编译器将拒绝加载它，除非 ``/etc/`` 也是允许的路径之一。

.. index:: ! remapping; import, ! import; remapping, ! remapping; context, ! remapping; prefix, ! remapping; target
.. _import-remapping:

导入重映射
================

导入重映射允许您将导入重定向到虚拟文件系统的不同位置。
该机制通过改变导入路径和源单元名称之间的转换来工作。
例如，您可以设置一个重映射，使任何从虚拟目录 ``github.com/ethereum/dapp-bin/library/``
的导入被视为从 ``dapp-bin/library/`` 导入。

您可以通过指定 *context* 来限制重映射的范围。
这允许创建仅适用于特定库或特定文件中的导入的重映射。
如果没有context关键字指定，重映射将应用于虚拟文件系统中所有文件中的每个匹配的导入。

导入重映射的形式为 ``context:prefix=target``：

- ``context`` 必须与包含导入文件的源单元名称的开头相匹配。
- ``prefix``  必须与导入的源单元名称的开头相匹配。
- ``target`` 是前缀被替换的值。

例如，如果您在本地克隆 https://github.com/ethereum/dapp-bin/ 到 ``/project/dapp-bin``，
并用以下命令运行编译器：

.. code-block:: bash

    solc github.com/ethereum/dapp-bin/=dapp-bin/ --base-path /project source.sol

您可以在您的源文件中使用以下内容：

.. code-block:: solidity

    import "github.com/ethereum/dapp-bin/library/math.sol"; // 源单元名称： dapp-bin/library/math.sol

编译器将在VFS的 ``dapp bin/library/math.sol`` 下寻找该文件。
如果那里没有该文件，源单元名称将被传递给主机文件系统加载器，
然后它将在 ``/project/dapp-bin/library/iterable_mapping.sol`` 中寻找。

.. warning::

    关于重映射的信息被存储在合约元数据中。
    由于编译器产生的二进制文件中嵌入了元数据的哈希值，对重映射的任何修改都会导致不同的字节码。

    由于这个原因，您应该注意不要在重映射目标中包含任何本地信息。
    例如，如果您的库位于 ``/home/user/packages/mymath/math.sol``，
    像 ``@math/=/home/user/packages/mymath/`` 这样的重映射会导致您的主目录被包含在元数据中。
    为了能够在不同的机器上用这样的重映射重现相同的字节码，
    您需要在VFS和（如果您依赖主机文件系统加载器）主机文件系统中重新创建您的本地目录结构。

    为了避免元数据中嵌入您的本地目录结构，建议将包含库的目录指定为 *include paths*。
    例如，在上面的例子中， ``--include-path /home/user/packages/`` 会让您使用以 ``mymath/`` 开始的导入。
    与重映射不同，该选项本身不会使 ``mymath`` 显示为 ``@math``，
    但这可以通过创建符号链接或重命名软件包子目录来实现。

作为一个更复杂的例子，假设您依赖一个使用旧版dapp-bin的模块，
您把它签出到 ``/project/dapp-bin_old``，那么您可以运行：

.. code-block:: bash

    solc module1:github.com/ethereum/dapp-bin/=dapp-bin/ \
         module2:github.com/ethereum/dapp-bin/=dapp-bin_old/ \
         --base-path /project \
         source.sol

这意味着 ``module2`` 的所有导入都指向旧版本，但 ``module1`` 的导入则指向新版本。

以下是关于重映射行为的详细规则：

#. **重新映射只影响导入路径和源单元名称之间的转换。**

   以任何其他方式添加到VFS的源单元名称不能被重新映射。
   例如，您在命令行上指定的路径和标准JSON中 ``sources.urls`` 中的路径不受影响。

   .. code-block:: bash

       solc /project/=/contracts/ /project/contract.sol # 源单元名称： /project/contract.sol

   在上面的例子中，编译器将从 ``/project/contract.sol`` 中加载源代码，
   并将其放在VFS中那个确切的源代码单元名下，而不是放在 ``/contract/contract.sol`` 中。

#. **上下文和前缀必须与源单元名称相匹配，而不是导入路径。**

   - 这意味着您不能直接重新映射 ``./`` 或 ``./``，因为它们在转译成源单元名称时被替换了，
     但您可以重新映射它们被替换的那部分名称：

     .. code-block:: bash

         solc ./=a/ /project/=b/ /project/contract.sol # 源单元名称： /project/contract.sol

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "./util.sol" as util; // 源单元名称： b/util.sol

   - 您不能重新映射基本路径或仅由导入回调内部添加的任何其他部分的路径。

     .. code-block:: bash

         solc /project/=/contracts/ /project/contract.sol --base-path /project # 源单元名称： contract.sol

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "util.sol" as util; // 源单元名称： util.sol

#. **目标直接插入源单元名称中，不一定是有效的路径。**

   - 只要导入回调能够处理它，它可以是任何东西。
     在主机文件系统加载器的情况下，这也包括相对路径。
     当使用JavaScript接口时，您甚至可以使用URL和抽象标识符，
     如果您的回调能够处理它们。

   - 重映射发生在相对导入已经被解析为源单元名称之后。
     这意味着以 ``./`` 和 ``./`` 开头的目标没有特殊含义，是相对于基本路径而不是源文件的位置。

   - 重映射目标没有被规范化，所以 ``@root/=./a/b//`` 将重映射
     ``@root/contract.sol`` 到 ``./a/b/contract.sol`` 而不是 ``a/b/contract.sol``。

   - 如果目标不以斜线结尾，编译器将不会自动添加一个斜线:

     .. code-block:: bash

         solc /project/=/contracts /project/contract.sol # 源单元名称： /project/contract.sol

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "/project/util.sol" as util; // 源单元名称： /contractsutil.sol

#. **上下文和前缀是匹配模式，匹配必须是精确的。**

   - ``a//b=c`` 不会匹配 ``a/b``。
   - 源单元名称没有被规范化，所以 ``a/b=c`` 也不会匹配 ``a//b``。
   - 文件和目录的部分名称是可以匹配。
     ``/newProject/con:/new=old`` 将匹配 ``/newProject/contract.sol``
     并将其重新映射到 ``oldProject/contract.sol``。

#. **最多只有一个重映射被应用于单个导入。**

   - 如果多个重映射与同一个源单元名称相匹配，则选择具有最长匹配前缀的那个。
   - 如果前缀相同，则选择最后指定的那个。
   - 重映射对其他重映射不起作用。例如 ``a=b b=c c=d`` 不会导致 ``a`` 被重映射到 ``d``。

#. **prefix不能为空，但context和target是可选的。**

   - 如果 ``target`` 是空字符串， ``prefix`` 将从导入路径中删除。
   - 空的 ``context`` 意味着重新映射适用于所有源单元中的所有导入。

.. index:: Remix IDE, file://

在导入中使用url
=====================

大多数URL前缀，如 ``https://`` 或 ``data://`` 在导入路径中没有特殊含义。
唯一的例外是 ``file://``，它被主机文件系统加载器从源单元名称中剥离出来。

在本地编译时，您可以使用导入重映射，用本地路径替换协议和域名部分：

.. code-block:: bash

    solc :https://github.com/ethereum/dapp-bin=/usr/local/dapp-bin contract.sol

注意前面的 ``:``，当重映射上下文为空时，这是必要的。
否则， ``https:`` 部分将被编译器解释为上下文。
