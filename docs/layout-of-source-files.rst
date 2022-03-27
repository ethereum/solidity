**********************
Solidity 源文件结构
**********************

源文件可以包含任意数量的
:ref:`contract 定义<contract_structure>`, import_ 指令,
:ref:`pragma 指令<pragma>` 和 :ref:`struct<structs>`,
:ref:`enum<enums>`, :ref:`function<functions>`, :ref:`error<errors>`
以及 :ref:`constant 变量<constants>` 的定义。

.. index:: ! license, spdx

SPDX 许可标识符
=======================

如果智能合约的源代码是公开的，就可以更好地建立对智能合约的信任。
由于提供源代码总是涉及到版权方面的法律问题，
Solidity编译器鼓励使用机器可读的 `SPDX 许可标识符 <https://spdx.org>`_ 。
每个源文件都应该以一个注释开始，表明其许可证

``// SPDX-License-Identifier: MIT``

编译器不会验证许可证是否属于 `SPDX许可的列表 <https://spdx.org/licenses/>`_，
但它确实包括在 :ref:`字节码元数据（bytecode metadata） <metadata>` 提供的字符串中。

如果您不想指定一个许可，或者源代码不是开源的，
请使用特殊值 ``UNLICENSED``。请注意， ``UNLICENSED`` （不允许使用，
不存在于SPDX许可证列表中）与 ``UNLICENSED`` （授予所有人所有权利）不同。
Solidity遵循 `npm 的推荐 <https://docs.npmjs.com/cli/v7/configuring-npm/package-json#license>`_。

提供这个注释并不能使你摆脱与许可有关的其他义务，
如必须在每个源文件中提到特定的许可头或原始版权人。

编译器可以在文件的任何位置识别该注释，
但建议把它放在文件的顶部。

关于如何使用SPDX许可证标识的更多信息可以在 `SPDX 网站 <https://spdx.org/ids-how>`_ 中找到。


.. index:: ! pragma

.. _pragma:

编译指示
==========

``pragma`` 关键字用于启用某些编译器特性或检查。
一个 pragma 指令始终是源文件的本地指令，
所以如果您想在整个项目中使用pragma指令，
您必须在您的所有文件中添加这个指令。
如果你 :ref:`import<import>` 另一个文件，
该文件的pragma指令 *不会* 自动应用于导入文件。

.. index:: ! pragma, version

.. _version_pragma:

版本编译指示
--------------

源文件可以（而且应该）用版本pragma指令来注释，
以拒绝用未来的编译器版本进行编译，因为这可能会引入不兼容的变化。
我们力图把这类变更做到尽可能小，
我们需要以一种当修改语义时必须同步修改语法的方式引入变更，
当然这有时候也难以做到。正因为如此，
至少在包含重大变化的版本中，通读一下更新日志总是一个好主意。
这些版本总是有 ``0.x.0`` 或 ``x.0.0`` 形式的版本。

版本编译指示使用如下： ``pragma solidity ^0.5.2;``

带有上述代码的源文件在0.5.2版本之前的编译器上不能编译，
在0.6.0版本之后的编译器上也不能工作（这第二个条件是通过使用 ``^`` 添加的）。
因为在 ``0.6.0`` 版本之前不会有任何重大的变化，
所以您可以确信您的代码是按照您的预期编译的。
上面例子中不固定编译器的具体版本号，因此编译器的补丁版也可以使用。

可以为编译器版本指定更复杂的规则，
这些规则与 `npm <https://docs.npmjs.com/cli/v6/using-npm/semver>`_ 使用相同的语法。

.. note::
  使用版本 pragma指令 *不会* 改变编译器的版本。
  它也 *不会* 启用或禁用编译器的功能。
  它只是指示编译器检查它的版本是否与编译指示所要求的版本一致。
  如果不匹配，编译器会发出一个错误。


ABI编码编译指示
----------------

通过使用 ``pragma abicoder v1`` 或 ``pragma abicoder v2`` ，
您可以选择ABI编码器和解码器的两种实现。

新的ABI编码器（v2）能够对任意的嵌套数组和结构进行编码和解码。
它可能产生不太理想的代码，并且没有得到像旧编码器那样多的测试，
但从 Solidity 0.6.0 起，它被认为是非实验性的。
你仍然必须使用 ``pragma abicoder v2;`` 明确激活它。
由于它将从Solidity 0.8.0 开始被默认激活，
所以可以选择使用 ``pragma abicoder v1;`` 来选择旧的编码器。

新编码器所支持的类型集是旧编码器所支持的类型的一个严格超集。
使用新编码器的合约可以与不使用新编码器的合约进行交互，没有任何限制。
只有当非 ``abicoder v2`` 的合约不试图进行需要解码新编码器支持的类型的调用时，
才有可能出现相反的情况。
编译器可以检测到这一点，并会发出一个错误。
只要为您的合同启用 ``abicoder v2`` ，就足以使错误消失。

.. note::
  这个编译指示适用于激活它的文件中定义的所有代码，
  无论这些代码最终在哪里结束。这意味着，
  一个合约的源文件被选择用ABI编码器v1编译，
  它仍然可以包含通过从另一个合约继承来使用新编码器的代码。
  如果新类型只在内部使用，而不是在外部函数签名中使用，
  这是被允许的。

.. note::
  到Solidity 0.7.4为止，可以通过使用 ``pragma experimental ABIEncoderV2``
  来选择ABI编码器v2，但不可能明确选择编码器v1，因为它是默认的。

.. index:: ! pragma, experimental

.. _experimental_pragma:

实验性编译指示
-------------------

第二个编译指示是实验性的编译指示。
它可以用来启用编译器或语言中尚未默认启用的功能。
目前支持以下实验性编译指示：


ABI编码器V2
~~~~~~~~~~~~

因为ABI编码器v2不再被认为是实验性的，
它可以通过 ``pragma abicoder v2`` （请见上文）从Solidity 0.7.4开始选择。

.. _smt_checker:

SMT检查器
~~~~~~~~~~

这个组件必须在构建 Solidity 编译器时被启用，
因此它不是在所有 Solidity 二进制文件中都可用。
:ref:`构建说明<smt_solvers_build>` 解释了如何激活这个选项。
它在大多数版本中为 Ubuntu PPA 版本激活，
但不用于Docker镜像、Windows二进制文件或静态构建的Linux二进制文件。
如果您在本地安装了SMT检查器并通过节点（而不是通过浏览器）运行solc-js，
可以通过 `smtCallback <https://github.com/ethereum/solc-js#example-usage with-smtsolver-callback>`_
为solc-js激活它。

如果您使用 ``pragma experimental SMTChecker;``，
那么你会得到额外的 :ref:`安全警告<formal_verification>`。
这些警告是通过查询SMT求解器获得的。
该组件还不支持Solidity语言的所有功能，可能会输出许多警告。
如果它报告不支持的功能，那么分析可能不完全正确。

.. index:: source file, ! import, module, source unit

.. _import:

导入其他源文件
==============

语法与语义
----------

Solidity 支持导入语句，以帮助模块化您的代码，
这些语句与 JavaScript 中可用的语句相似(从ES6开始)。
然而，Solidity并不支持 `默认导出 <https://developer.mozilla.org/en-US/docs/web/javascript/reference/statements/export#Description>`_
的概念。

在全局层面，您可以使用以下形式的导入语句：

.. code-block:: solidity

    import "filename";

``filename`` 部分被称为 *导入路径*。
该语句将所有来自 "filename" 的全局符号（以及在那里导入的符号）
导入到当前的全局范围（与ES6中不同，但对Solidity来说是向后兼容的）。
这种形式不建议使用，因为它不可预测地污染了命名空间。
如果您在 "filename" 里面添加新的顶层项目，
它们会自动出现在所有像这样从 "filename" 导入的文件中。
最好是明确地导入特定的符号。

下面的例子创建了一个新的全局符号 ``symbolName``，其成员均来自 ``"filename"`` 中全局符号；

.. code-block:: solidity

    import * as symbolName from "filename";

这意味着所有全局符号以 ``symbolName.symbol`` 的格式提供。

另一种语法不属于 ES6，但可能是有用的：

.. code-block:: solidity

  import "filename" as symbolName;

这条语句等同于 ``import * as symbolName from "filename";``。

如果有命名冲突，您可以在导入的同时重命名符号。
例如，下面的代码创建了新的全局符号 ``alias`` 和 ``symbol2``，
它们分别从 ``"filename"`` 里面引用 ``symbol1`` 和 ``symbol2``。

.. code-block:: solidity

    import {symbol1 as alias, symbol2} from "filename";

.. index:: virtual filesystem, source unit name, import; path, filesystem path, import callback, Remix IDE

导入路径
---------

为了能够在所有平台上支持可重复的构建，
Solidity 编译器必须抽象出存储源文件的文件系统的细节。
由于这个原因，导入路径并不直接指向主机文件系统中的文件。
相反，编译器维护一个内部数据库（ *虚拟文件系统* 或简称 *VFS* ），
每个源单元被分配一个唯一的 *源单元名称*，
这是一个不透明的、非结构化的标识。
在导入语句中指定的导入路径被转译成源单元名称，并用于在这个数据库中找到相应的源单元。

使用 :ref:`标准 JSON <compiler-api>` API，
可以直接提供所有源文件的名称和内容作为编译器输入的一部分。
在这种情况下，源单元的名称确实是任意的。
然而，如果你想让编译器自动查找并将源代码加载到VFS中，
您的源单元名称需要以一种结构化的方式，使 :ref:`回调引用 <import-callback>` 能够定位它们。
当使用命令行编译器时，默认的回调引用只支持从主机文件系统加载源代码，
这意味着您的源单元名称必须是路径。一些环境提供了自定义的回调，其用途更广。
例如， `Remix IDE <https://remix.ethereum.org/>`_ 提供了一个可以让你
`从HTTP、IPFS和Swarm URL导入文件，或者直接引用NPM注册表中的包 <https://remix-ide.readthedocs.io/en/latest/import.html>`_。

关于虚拟文件系统和编译器使用的路径解析逻辑的完整描述，请参见 :ref:`路径解析 <path-resolution>`。

.. index:: ! comment, natspec

注释
========

可以使用单行注释（ ``//`` ）和多行注释（ ``/*...*/`` ）

.. code-block:: solidity

    // 这是一个单行注释。

    /*
    这是一个
    多行注释。
    */

.. note::
  单行注释由UTF-8编码中的任何单码行结束符（LF、VF、FF、CR、NEL、LS或PS）结束。
  终结符在注释之后仍然是源代码的一部分，
  所以如果它不是一个ASCII符号（这些是NEL、LS和PS），将导致解析器错误。

此外，还有一种注释叫做NatSpec注释，在 :ref:`格式指南<style_guide_natspec>` 中详细说明。
它们用三斜线（ ``///`` ）或双星号块（ ``/** ... */`` ）来写，
它们应该直接用在函数声明或语句的上方。
