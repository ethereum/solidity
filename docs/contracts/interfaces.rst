.. index:: ! contract;interface, ! interface contract

.. _interfaces:

**********************
接口（interface）合约
**********************

接口（interface）合约类似于抽象（abstract）合约，但是它们不能实现任何函数。并且还有进一步的限制：

- 它们不能继承其他合约，但是它们可以继承其他接口合约。
- 在接口合约中所有声明的函数必须是 external 类型的，即使它们在合约中是 public 类型的。
- 它们不能声明构造函数。
- 它们不能声明状态变量。
- 它们不能声明修饰器。

将来可能会解除这些里的某些限制。

接口合约基本上仅限于合约 ABI 可以表示的内容，
并且 ABI 和接口合约之间的转换应该不会丢失任何信息。

接口合约由它们自己的关键字表示：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    interface Token {
        enum TokenType { Fungible, NonFungible }
        struct Coin { string obverse; string reverse; }
        function transfer(address recipient, uint amount) external;
    }

就像继承其他合约一样，合约可以继承接口合约。

所有在接口合约中声明的函数都是隐式的 ``virtual`` 的类型，
任何重载它们的函数都不需要 ``override`` 关键字。
这并不自动意味着一个重载的函数可以被再次重载--这只有在重载的函数被标记为 ``virtual`` 时才可能。

接口合约可以从其他接口合约继承。这与普通的继承有着相同的规则。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    interface ParentA {
        function test() external returns (uint256);
    }

    interface ParentB {
        function test() external returns (uint256);
    }

    interface SubInterface is ParentA, ParentB {
        // 必须重新定义test，以便断言父类的含义是兼容的。
        function test() external override(ParentA, ParentB) returns (uint256);
    }

在接口合约和其他类似合约的结构中定义的类型可以从其他合约中访问： ``Token.TokenType`` 或 ``Token.Coin``。

.. 警告:

    接口合约从 :doc:`Solidity 0.5.0 <050-breaking-changes>` 开始支持 ``enum`` 类型，
    请确保pragma版本至少指定这个版本。
