Solidity
========

Solidity是一门为实现智能合约而创建的面向对象的高级编程语言。 智能合约是管理以太坊中账户行为的程序。

Solidity是一种 `带花括号的语言 <https://en.wikipedia.org/wiki/List_of_programming_languages_by_type#Curly-bracket_languages>`_.
这门语言受到了 C++，Python 和 Javascript 的影响，并且旨在针对以太坊虚拟机（EVM）。
您可以在 :doc:`语言的影响 <language-influences>` 这一章节中找到更多关于 Solidity 受到哪些语言启发的细节。

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

.. note::

  您可以通过点击左下角的版本号弹出的菜单来选择首选的下载格式来下载该文档的 PDF、HTML 或 Epub 格式。

开始入门
---------------

**1. 理解合约基础概念**

如果您不熟悉智能合约的概念，我们建议您从探索 “智能合约简介“ 部分开始，其中包括：

* :ref:`一个用Solidity编写的简单的智能合约实例 <simple-smart-contract>` 。
* :ref:`区块链基础 <blockchain-basics>`。
* :ref:`以太坊虚拟机 <the-ethereum-virtual-machine>`。

**2. 开始了解Solidity**

一旦熟悉了基础概念，
我们推荐您阅读 :doc:`"Solidity实例" <solidity-by-example>` 和 “语言描述“ 部分来了解该语言的核心概念。

**3. 安装Solidity编译器**

有多种方式来安装Solidity编译器,
只需选择您喜欢的方式，并按照 :ref:`安装手册 <installing-solidity>` 上的步骤进行安装。

.. note::
  您可以使用 `Remix IDE <https://remix.ethereum.org>`_ 在浏览器中直接运行示例代码。
  Remix 是一个基于 Web 浏览器的 IDE，允许您编写，部署和管理 Solidity 智能合约，
  而无需在本地安装 Solidity。

.. warning::
    因为软件是人编写的，就会有bug。在编写智能合约时，您应该遵循既定的软件开发最佳实践。
    这包括代码审查，测试，审计和正确性证明。
    智能合约用户有时比他们的作者对代码更有信心，
    区块链和智能合约有自己独特的问题需要注意，
    所以在生产代码工作之前，确保您阅读 :ref:`security_considerations` 部分。

**4. 了解更多**

如果您想了解更多关于在以太坊上构建去中心化应用程序的信息，
`以太坊开发者资源 <https://ethereum.org/en/developers/>`_ 可以帮助您进一步了解关于以太坊的一般文件，
以及大量的教程，工具和开发框架的选择。

如果您有任何问题，您可以尝试在 `Ethereum StackExchange <https://ethereum.stackexchange.com/>`_，
或我们的 `Gitter频道 <https://gitter.im/ethereum/solidity/>`_ 上搜索答案或提问。

.. _translations:

翻译
------------

社区志愿者帮助将这些文件翻译成各种语言。
它们有不同程度的完整性和最新性。但英文版本作为主要参考。

.. note::

   我们最近建立了一个新的GitHub组织和翻译工作流程，以帮助简化社区工作。
   请参考 `翻译指南 <https://github.com/solidity-docs/translation-guide>`_，
   了解如何为社区的翻译工作做出贡献。

* `法语 <https://solidity-fr.readthedocs.io>`_ （正在进行）
* `意大利语 <https://github.com/damianoazzolini/solidity>`_ （正在进行）
* `日语 <https://solidity-jp.readthedocs.io>`_
* `韩语 <https://solidity-kr.readthedocs.io>`_ （正在进行）
* `俄语 <https://github.com/ethereum/wiki/wiki/%5BRussian%5D-%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Solidity>`_ (已过时)
* `简体中文 <https://learnblockchain.cn/docs/solidity/>`_ （正在进行）
* `西班牙语 <https://solidity-es.readthedocs.io>`_
* `土耳其语 <https://github.com/denizozzgur/Solidity_TR/blob/master/README.md>`_ (完成部分)

目录
========

:ref:`Keyword Index <genindex>`, :ref:`Search Page <search>`

.. toctree::
   :maxdepth: 2
   :caption: Basics

   introduction-to-smart-contracts.rst
   installing-solidity.rst
   solidity-by-example.rst

.. toctree::
   :maxdepth: 2
   :caption: Language Description

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
   :caption: Compiler

   using-the-compiler.rst
   analysing-compilation-output.rst
   ir-breaking-changes.rst

.. toctree::
   :maxdepth: 2
   :caption: Internals

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
   :caption: Additional Material

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
