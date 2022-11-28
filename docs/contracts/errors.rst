.. index:: ! error, revert, ! selector; of an error
.. _errors:

*******************************
错误和恢复语句
*******************************

Solidity 中的错误提供了一种方便且省gas的方式来向用户解释为什么一个操作会失败。
它们可以被定义在合约内部和外部（包括接口合约和库合约）。

它们必须与 :ref:`恢复语句 <revert-statement>` 一起使用，
它导致当前调用中的所有变化被恢复，并将错误数据传回给调用者。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    /// 转账的余额不足。需要 `required` 数量但只有 `available` 数量可用。
    /// @param 可用的余额。
    /// @param 需要要求的转帐金额。
    error InsufficientBalance(uint256 available, uint256 required);

    contract TestToken {
        mapping(address => uint) balance;
        function transfer(address to, uint256 amount) public {
            if (amount > balance[msg.sender])
                revert InsufficientBalance({
                    available: balance[msg.sender],
                    required: amount
                });
            balance[msg.sender] -= amount;
            balance[to] += amount;
        }
        // ...
    }

错误不能被重载或覆盖，但是可以被继承。
只要作用域不同，同一个错误可以在多个地方定义。
错误的实例只能使用 ``revert`` 语句创建。

错误会创建数据，然后通过还原操作传递给调用者，
使其返回到链下组件或在 :ref:`try/catch 语句 <try-catch>` 中捕获它。
需要注意的是，一个错误只能在来自外部调用时被捕获，
发生在内部调用或同一函数内的还原不能被捕获。

如果您不提供任何参数，错误只需要四个字节的数据，
您可以像上面一样使用 :ref:`NatSpec 语法 <natspec>` 来进一步解释错误背后的原因，
这并不存储在链上。这使得这同时也是一个非常便宜和方便的错误报告功能。

更具体地说，一个错误实例在被ABI编码时，
其方式与对相同名称和类型的函数的调用相同，
然后作为 ``revert`` 操作码的返回数据。
这意味着数据由一个4字节的选择器和 :ref:`ABI编码 <abi>` 数据组成。
选择器由错误类型的签名的keccak256-hash的前四个字节组成。

.. note::
    一个合约有可能因为同名的不同错误而恢复，
    甚至因为在不同地方定义的错误而使调用者无法区分。
    对于外部来说，即ABI，只有错误的名称是相关的，而不是定义它的合约或文件。

如果您能定义 ``error Error(string)``，
那么语句 ``require(condition, "description");``
将等同于 ``if (!condition) revert Error("description")``。
但是请注意， ``Error`` 是一个内置类型，不能在用户提供的代码中定义。

同样，一个失败的 ``assert`` 或类似的条件将以一个内置的 ``Panic(uint256)`` 类型的错误来恢复。

.. note::
    错误数据应该只被用来指示失败，而不是作为控制流的手段。
    原因是内部调用的恢复数据默认是通过外部调用链传播回来的。
    这意味着内部调用可以 ”伪造” 恢复数据，使它看起来像是来自调用它的合约。

错误类型的成员
=================

- ``error.selector``： 一个包含错误类型的选择器的 ``bytes4`` 值。
