Solidity
========

.. warning::

  You are reading a community translation of the Solidity documentation. The Solidity team
  can give no guarantees on the quality and accuracy of the translations provided.
  The English reference version is and will remain the only officially supported version
  by the Solidity team and will always be the most accurate and most up-to-date one.
  When in doubt, please always refer to the `English (original) documentation <https://docs.soliditylang.org/en/latest/>`_.

Solidity是一门为实现智能合约而创建的面向对象的高级编程语言。 智能合约是管理以太坊中账户行为的程序。

Solidity 是一种面向以太坊虚拟机 (EVM) 的 `带花括号的语言 <https://en.wikipedia.org/wiki/List_of_programming_languages_by_type#Curly-bracket_languages>`_。
它受 C++，Python 和 JavaScript 的影响。您可以在  :doc:`语言的影响因素 <language-influences>` 部分中找到更多有关 Solidity 受哪些语言启发的细节。

Solidity 是静态类型语言，支持继承，库和复杂的用户自定义的类型以及其他特性。

下面您将会看到，使用 Solidity，您可以创建用于投票、众筹、秘密竞价（盲拍）以及多重签名钱包等用途的合约。

当开发智能合约时，您应该使用最新版本的Solidity。除某些特殊情况之外，只有最新版本才会收到
`安全修复 <https://github.com/ethereum/solidity/security/policy#supported-versions>`_。
此外，重大的变化以及新功能会定期引入。
目前，我们使用 0.y.z 版本号 `来表明这种快速的变化 <https://semver.org/#spec-item-4>`_。

.. warning::

  Solidity最近发布了0.8.x版本，该版本引入了许多重大更新。 清务必阅读 :doc:`完整列表 <080-breaking-changes>`。

始终欢迎改进 Solidity 或此文档的想法,
请阅读我们的 :doc:`贡献者指南 <contributing>` 以了解更多细节。

.. Hint::

  您可以通过点击左下角的版本号弹出的菜单来选择首选的下载格式来下载该文档的 PDF，HTML 或 Epub 格式。


入门指南
---------------

**1. 了解智能合约基础知识**

如果您是智能合约概念的新手，我们建议您从深入了解 "智能合约介绍" 部分开始，其中包括：

* 用 Solidity 编写的 :ref:`一个简单的智能合约例子 <simple-smart-contract>`。
* :ref:`区块链基础知识 <blockchain-basics>`.
* :ref:`以太坊虚拟机 <the-ethereum-virtual-machine>`.

**2. 了解 Solidity**

一旦您熟悉了基础知识，我们建议您阅读 :doc:`"Solidity 示例" <solidity-by-example>`
和 “语言描述” 部分，以了解该语言的核心概念。

**3.安装 Solidity 编译器**

有多种方法可以安装 Solidity 编译器，
只需选择您喜欢的选项，并按照 :ref:`安装页面 <installing-solidity>` 上提供的步骤操作即可。

.. hint::
  您可以通过 `Remix IDE <https://remix.ethereum.org>`_ 在浏览器中直接尝试代码示例。
  Remix 是一个基于网络浏览器的IDE，允许您编写，部署和管理Solidity智能合约，
  无需在本地安装 Solidity。

.. warning::
    由于人类编写的软件可能会存在 bug，
    因此在编写智能合约时应遵循软件开发的最佳实践。
    这包括代码审查，测试，审计和正确性证明。
    智能合约用户有时对代码的信心甚至超过了作者，
    区块链和智能合约也存在独特的问题，
    因此在开始编写生产代码之前，请确保您已阅读
    :ref:`安全考虑` 部分。

**4. 了解更多**

如果您想更深入了解如何在以太坊上构建去中心化应用，
`以太坊开发者资源 <https://ethereum.org/en/developers/>`_ 可以为您提供有关以太坊的更多文档，
以及各种教程、工具和开发框架。

如果您有任何问题，可以在 `以太坊 StackExchange <https://ethereum.stackexchange.com/>`_ 上寻找答案，
或者在我们的 `Gitter 频道 <https://gitter.im/ethereum/solidity/>`_。

.. _translations:

翻译
------------

社区贡献者帮助将本文档翻译成多种语言。
请注意，这些翻译的完整度和及时性各不相同。
因此英文版才是参考的标准。

您可以通过点击左下角的语言切换器来切换语言。
在弹出的菜单中，选择您需要的语言即可切换。

* `法语 <https://docs.soliditylang.org/fr/latest/>`_
* `印度尼西亚语 <https://github.com/solidity-docs/id-indonesian>`_
* `波斯语 <https://github.com/solidity-docs/fa-persian>`_
* `日语 <https://github.com/solidity-docs/ja-japanese>`_
* `韩语 <https://github.com/solidity-docs/ko-korean>`_
* `简体中文 <https://github.com/solidity-docs/zh-cn-chinese/>`_

.. note::

   我们最近建立了一个新的 GitHub 组织和翻译流程，以帮助简化社区的翻译工作。
   有关如何开始新的翻译语言或为社区翻译作出贡献的信息，
   请参阅 `翻译指南 <https://github.com/solidity-docs/translation-guide>`_。

目录
========

:ref:`关键字索引 <genindex>`, :ref:`搜索页面 <search>`

.. toctree::
   :maxdepth: 2
   :caption: 基础知识

   introduction-to-smart-contracts.rst
   installing-solidity.rst
   solidity-by-example.rst

.. toctree::
   :maxdepth: 2
   :caption: 语言描述

   layout-of-source-files.rst
   structure-of-a-contract.rst
   types.rst
   units-and-global-variables.rst
   control-structures.rst
   contracts.rst
   assembly.rst
   cheatsheet.rst
   grammar.rst

.. toctree::
   :maxdepth: 2
   :caption: 编译器

   using-the-compiler.rst
   analysing-compilation-output.rst
   ir-breaking-changes.rst

.. toctree::
   :maxdepth: 2
   :caption: 内部说明

   internals/layout_in_storage.rst
   internals/layout_in_memory.rst
   internals/layout_in_calldata.rst
   internals/variable_cleanup.rst
   internals/source_mappings.rst
   internals/optimizer.rst
   metadata.rst
   abi-spec.rst

.. toctree::
   :maxdepth: 2
   :caption: 补充材料

   050-breaking-changes.rst
   060-breaking-changes.rst
   070-breaking-changes.rst
   080-breaking-changes.rst
   natspec-format.rst
   security-considerations.rst
   smtchecker.rst
   resources.rst
   path-resolution.rst
   yul.rst
   style-guide.rst
   common-patterns.rst
   bugs.rst
   contributing.rst
   brand-guide.rst
   language-influences.rst
