.. index:: ! constant

.. _constants:

**************************************
Constant 和 Immutable 状态变量
**************************************

状态变量可以被声明为 ``constant`` 或 ``immutable``。
在这两种情况下，变量在合约构建完成后不能被修改。
对于 ``constant`` 变量，其值必须在编译时固定，
而对于 ``immutable`` 变量，仍然可以在构造时分配。

也可以在文件级别定义 ``constant`` 变量。

编译器并没有为这些变量预留存储，它们的每次出现都会被替换为相应的常量表达式。

与普通的状态变量相比，常量变量（constant）和不可改变的变量（immutable）的气体成本要低得多。
对于常量变量，分配给它的表达式被复制到所有访问它的地方，并且每次都要重新评估，
这使得局部优化成为可能。不可变的变量在构造时被评估一次，其值被复制到代码中所有被访问的地方。
对于这些值，要保留32个字节，即使它们可以装入更少的字节。由于这个原因，常量值有时会比不可变的值更便宜。

目前，并非所有的常量和不可变量的类型都已实现。
唯一支持的类型是 :ref:`字符串类型 <strings>` （仅用于常量）和 :ref:`值类型 <value-types>`。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.4;

    uint constant X = 32**22 + 8;

    contract C {
        string constant TEXT = "abc";
        bytes32 constant MY_HASH = keccak256("abc");
        uint immutable decimals;
        uint immutable maxBalance;
        address immutable owner = msg.sender;

        constructor(uint decimals_, address ref) {
            decimals = decimals_;
            // 对不可变量的赋值甚至可以访问一些全局属性。
            maxBalance = ref.balance;
        }

        function isBalanceTooHigh(address other) public view returns (bool) {
            return other.balance > maxBalance;
        }
    }


Constant
========

对于 ``constant`` 变量，其值在编译时必须是一个常量，并且必须在变量声明的地方分配。
任何访问存储、区块链数据（例如： ``block.timestamp``, ``address(this).balance`` 或 ``block.number``）
或执行数据（ ``msg.value`` 或 ``gasleft()``）或者调用外部合约的表达式都是不允许的。
但可能对内存分配产生副作用的表达式是允许的，但那些可能对其他内存对象产生副作用的表达式是不允许的。
内置函数 ``keccak256``， ``sha256``， ``ripemd160``， ``ecrecover``， ``addmod`` 和 ``mulmod``
是允许的（尽管除了 ``keccak256``，它们确实调用了外部合约）。

允许在内存分配器上产生副作用的原因是，
它应该可以构建复杂的对象，比如说查找表。
这个功能现在还不能完全使用。

Immutable
=========

声明为 ``immutable`` 的变量比声明为 ``constant`` 的变量受到的限制要少一些。
不可变的变量可以在合约的构造函数中或在声明时被分配一个任意的值。
它们只能被分配一次，并且从那时起，即使在构造时间内也可以被读取。

编译器生成的合约创建代码将在其返回之前修改合约的运行时代码，
用分配给它们的值替换所有对不可变量的引用。
当您将编译器生成的运行时代码与实际存储在区块链中的代码进行比较时，这一点很重要。

.. note::
  在声明时被分配的不可变量只有在合约的构造函数执行时才会被视为初始化。
  这意味着您不能在内联中用一个依赖于另一个不可变量的值来初始化不可变量。
  然而，您可以在合约的构造函数中这样做。

  这是对状态变量初始化和构造函数执行顺序的不同解释的一种保障，特别是在继承方面。
