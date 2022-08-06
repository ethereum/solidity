.. _natspec:

##############
风格指南
##############

Solidity合约可以使用一种特殊形式的注释来为函数，返回变量等提供丰富的文档。
这种特殊形式被命名为Ethereum自然语言规范格式（NatSpec）。

.. note::

  NatSpec是受 `Doxygen <https://en.wikipedia.org/wiki/Doxygen>`_ 的启发。
  虽然它使用Doxygen风格的注释和标签，但并不打算与Doxygen保持严格的兼容性。
  请仔细检查下面列出的支持的标签。

该文件被划分为以开发人员为中心的信息和面向最终用户的信息。
这些信息可以在终端用户（人类）与合约交互（即签署交易）时显示给他们。

建议 Solidity 合约使用 NatSpec 对所有公共接口（ABI中的一切）进行完全地注释。

NatSpec 包括智能合约作者将使用的注释的格式，
这些注释可被 Solidity 编译器理解。
下面还详细介绍了 Solidity 编译器的输出，
它将这些注释提取为机器可读的格式。

NatSpec 也可以包括第三方工具使用的注释。
这些最可能是通过 ``@custom:<name>`` 标签完成的，
一个好的用例是分析和验证工具就是如此。

.. _header-doc-example:

文档示例
=====================

文档可以通过使用 Doxygen 符号格式来嵌入到每个 ``contract``， ``interface``， ``library``，
``function`` 和 ``event`` 之上。在 NatSpec 中， ``public`` 状态变量等同于 ``function``。

-  对于Solidity，您可以选择 ``///`` 用于单行注释
   或以 ``/**`` 开始，并以 ``*/`` 结束的符号用于多行注释

-  对于Vyper来说，使用 ``""""`` 缩进到内部内容来裸注释（译者注：无标记符号注释）。
   参见 `Vyper 文档 <https://vyper.readthedocs.io/en/latest/natspec.html>`__。

下面的例子显示了一个合约和一个使用所有可用标记的函数。

.. note::

  Solidity 编译器只在标签是外部或公共的情况下才进行解析。
  但也欢迎您为您的内部和私有函数使用类似的注释，不过这些不会被解析。

  这在未来可能会发生变化。

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.2 < 0.9.0;

    /// @title 树的模拟器
    /// @author Larry A. Gardner
    /// @notice 您只能将此合约用于最基本的模拟。
    /// @dev 目前所有的函数调用都是在没有副作用的情况下实现的
    /// @custom:experimental 这是一个实验性的合约。
    contract Tree {
        /// @notice 计算活体树木的树龄，按四舍五入计算
        /// @dev Alexandr N. Tetearing 算法可以提高精确度
        /// @param rings 树龄学样本的环数
        /// @return 树龄（岁），部分年份四舍五入
        function age(uint256 rings) external virtual pure returns (uint256) {
            return rings + 1;
        }

        /// @notice 返回该树的叶子数量。
        /// @dev 在此只是返回了一个固定的数字。
        function leaves() external virtual pure returns(uint256) {
            return 2;
        }
    }

    contract Plant {
        function leaves() external virtual pure returns(uint256) {
            return 3;
        }
    }

    contract KumquatTree is Tree, Plant {
        function age(uint256 rings) external override pure returns (uint256) {
            return rings + 2;
        }

        /// 返回这种特定类型的树的叶子数量。
        /// @inheritdoc Tree 合约
        function leaves() external override(Tree, Plant) pure returns(uint256) {
            return 3;
        }
    }

.. _header-tags:

标签
====

所有标签都是可选的。下表解释了每个 NatSpec 标签的目的和它可能被使用的地方。
有一种特殊情况，如果没有使用标签，那么 Solidity 编译器将以同样的方式进行 ``///`` 或 ``/**`` 注释，
如同它被标记为 ``@notice``。

=============== ====================================================================================== =============================
标签                                                                                                    应用于
=============== ====================================================================================== =============================
``@title``      一个应该描述合约/接口的标题                                                                contract, library, interface
``@author``     作者的名字                                                                              contract, library, interface
``@notice``     向终端用户解释这个东西的作用                                                               contract, library, interface, function, public state variable, event
``@dev``        向开发人员解释任何额外的细节                                                               contract, library, interface, function, state variable, event
``@param``      就像在Doxygen中一样记录一个参数（必须在参数名之后）                                           function, event
``@return``     记录一个合约的函数的返回变量                                                                function, public state variable
``@inheritdoc`` 从基本函数中复制所有缺失的标签（必须在合约名称之后）                                            function, public state variable
``@custom:...`` 自定义标签，语义由应用程序定义                                                              everywhere
=============== ====================================================================================== =============================

如果您的函数返回多个值，如 ``(int quotient, int remainder)``
那么使用多个 ``@return`` 语句，格式与 ``@param`` 语句相同。

自定义标签以 ``@custom:`` 开头，后面必须有一个或多个小写字母或连字符。
然而，它不能以连字符开始。它们可以在任何地方使用，是开发者文档的一部分。

.. _header-dynamic:

动态表达方式
-------------------

Solidity 编译器将通过 NatSpec 文档从您的 Solidity 源代码传递到本指南所述的 JSON 输出。
此 JSON 输出的使用者，例如最终用户的客户端软件，可以直接将其呈现给最终用户，或者它可以应用一些预处理。

例如，一些客户端软件会呈现为：

.. code:: Solidity

   /// @notice 这个函数将使 `a` 乘以7

对终端用户来说，是：

.. code:: text

    这个函数将10乘以7

如果一个函数被调用，并且输入的 ``a`` 被赋值为10。

指定这些动态表达式超出了 Solidity 文档的范围，您可以在
`radspec 项目 <https://github.com/aragon/radspec>`__ 阅读更多内容。

.. _header-inheritance:

继承说明
-----------------

没有NatSpec的函数将自动继承其基函数的文档。这方面的例外情况是：

* 当参数名称不同时。
* 当有不止一个的基础函数时。
* 当有一个明确的 ``@inheritdoc`` 标签，指定了应该使用哪个合约来继承。

.. _header-output:

文件输出
====================

当被编译器解析时，像上面例子中的文档将产生两个不同的JSON文件。
一个是为了让终端用户在执行函数时作为通知使用，另一个是为了让开发人员使用。

如果上述合约被保存为 ``ex1.sol``，那么您可以用以下方法生成文档：

.. code::

   solc --userdoc --devdoc ex1.sol

输出如下。

.. note::
    从Solidity 0.6.11版开始，NatSpec输出也包含一个 ``version（版本号）`` 和一个 ``kind（种类）`` 字段。
    目前， ``version`` 被设置为 ``1``， ``kind`` 必须是 ``user（用户）`` 或 ``dev（开发者）`` 之一。
    在未来，有可能会引入新的版本，淘汰旧的版本。

.. _header-user-doc:

用户文档
------------------

上述文档将产生以下用户文档 JSON 文件作为输出：

.. code::

    {
      "version" : 1,
      "kind" : "user",
      "methods" :
      {
        "age(uint256)" :
        {
          "notice" : "计算活体树木的树龄，按四舍五入计算"
        }
      },
      "notice" : "您只能将此合约用于最基本的模拟。"
    }

请注意，找到方法的关键是 :ref:`合约 ABI <abi_function_selector>` 中定义的函数的标准签名，
而不是简单的函数名称。

.. _header-developer-doc:

开发者文档
-----------------------

除了用户文档文件，还应该产生一个开发者文档的JSON文件，看起来应该是这样的：

.. code::

    {
      "version" : 1,
      "kind" : "dev",
      "author" : "Larry A. Gardner",
      "details" : "目前所有的函数调用都是在没有副作用的情况下实现的",
      "custom:experimental" : "这是一个实验性的合约。",
      "methods" :
      {
        "age(uint256)" :
        {
          "details" : "Alexandr N. Tetearing 算法可以提高精确度",
          "params" :
          {
            "rings" : "树龄学样本的环数"
          },
          "return" : "树龄（岁），部分年份四舍五入"
        }
      },
      "title" : "树的模拟器"
    }
