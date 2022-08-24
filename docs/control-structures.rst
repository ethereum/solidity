###################
表达式和控制结构
###################

.. index:: ! parameter, parameter;input, parameter;output, function parameter, parameter;function, return variable, variable;return, return


.. index:: if, else, while, do/while, for, break, continue, return, switch, goto

控制结构
===================

大多数从大括号语言中知道的控制结构都可以在Solidity中使用：

有： ``if``， ``else``，  ``while``， ``do``， ``for``， ``break``， ``continue``， ``return``，
这些在 C 或者 JavaScript 中表达相同语义的关键词。

Solidity也支持 ``try`` / ``catch`` 形式的语句的异常处理，
但只适用于 :ref:`外部函数调用 <external-function-calls>` 和合约创建调用。
可以使用 :ref:`恢复状态 <revert-statement>` 来创建错误。

条件句 *不能* 省略括号，但单句体周围可以省略大括号。

请注意，没有像C和JavaScript那样从非布尔类型到布尔类型的类型转换，
所以 ``if (1) { ... }`` 在Solidity *不是* 有效的。

.. index:: ! function;call, function;internal, function;external

.. _function-calls:

函数调用
=========

.. _internal-function-calls:

内部函数调用
--------------

当前合约中的函数可以直接（“从内部”）调用，也可以递归调用，就像下边这个荒谬的例子一样：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;

    // 这会有一个警告
    contract C {
        function g(uint a) public pure returns (uint ret) { return a + f(); }
        function f() internal pure returns (uint ret) { return g(7) + f(); }
    }

这些函数调用在EVM内部被转化为简单的跳转。
这样做的效果是，当前的内存不会被清空，也就是说，
将内存引用传递给内部调用的函数是非常有效的。
但只有同一合约实例的函数可以被内部调用。

您还是应该避免过度的递归调用，因为每个内部函数的调用都会占用至少一个堆栈槽，而可用的堆栈槽只有1024个。

.. _external-function-calls:

外部函数调用
-------------

函数也可以使用 ``this.g(8);`` 和 ``c.g(2);`` 符号来调用，
其中 ``c`` 是一个合约实例， ``g`` 是属于 ``c`` 的函数。
通过这两种方式调用函数 ``g`` 会导致它被 “外部” 调用，
使用消息调用而不是直接通过跳转。
请注意，对 ``this`` 的函数调用不能在构造函数中使用，因为实际的合约还没有被创建。

其他合约的函数必须被外部调用。对于一个外部调用，
所有的函数参数都必须被拷贝到内存中。

.. note::
    从一个合约到另一个合约的函数调用并不创建自己的交易，它是作为整个交易的一部分的消息调用。

当调用其他合约的函数时，您可以用特殊的选项 ``{value: 10, gas: 10000}`` 指定随调用发送的Wei或气体（gas）数量。
请注意，不鼓励明确指定气体值，因为操作码的气体成本可能在未来发生变化。
您发送给合约的任何Wei都会被添加到该合约的总余额中：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    contract InfoFeed {
        function info() public payable returns (uint ret) { return 42; }
    }

    contract Consumer {
        InfoFeed feed;
        function setFeed(InfoFeed addr) public { feed = addr; }
        function callFeed() public { feed.info{value: 10, gas: 800}(); }
    }

您需要对 ``info`` 函数使用修饰符 ``payable``，
因为不这样的话， ``value`` 选项则不可用。

.. warning::
  注意 ``feed.info{value: 10, gas: 800}`` 只在本地设置 ``value`` 和随函数调用发送的 ``gas`` 数量，
  最后的括号执行实际调用。所以 ``feed.info{value: 10, gas: 800}`` 不会调用函数，
  ``value`` 和 ``gas`` 的设置也会丢失，
  只有 ``feed.info{value: 10, gas: 800}()`` 执行了函数调用。

由于EVM认为对一个不存在的合约的调用总是成功的，
Solidity使用 ``extcodesize`` 操作码来检查即将被调用的合约是否真的存在（它包含代码），
如果不存在就会引起异常。如果返回数据将在调用后被解码，
则跳过该检查，因此ABI解码器将捕获不存在的合约的情况。

请注意，这个检查在 :ref:`低级调用 <address_related>` 的情况下不执行，
这些调用是对地址而不是合约实例进行操作。

.. note::
    在对 :ref:`预编译合约 <precompiledContracts>` 使用高级调用时要小心，
    因为根据上述逻辑，编译器认为它们不存在，即使它们执行代码并可以返回数据。

如果被调用的合约本身抛出异常或超出了gas值，函数调用也会引起异常。

.. warning::
    与另一个合约的任何互动都会带来潜在的危险，
    特别是当合约的源代码事先不知道的时候。
    当前的合约将控制权交给了被调用的合约，而这有可能做任何事情。
    即使被调用的合约继承自一个已知的父合约，
    继承的合约也只需要有一个正确的接口。
    然而，合约的实现完全可以是任意的，因此这会带来危险。
    此外，要做好准备，以防它调用到您系统中的其他合约，
    甚至在第一次调用返回之前就回到调用合约中。
    这意味着被调用的合约可以通过这个函数改变调用合约的状态变量。
    编写您的函数时，例如，对外部函数的调用发生在对您的合约中的状态变量的任何改变之后，
    这样您的合约就不会受到重入性漏洞的攻击。

.. note::
    在 Solidity 0.6.2 之前，指定以太值和气体值的推荐方法是
    使用 ``f.value(x).gas(g)()``。这在Solidity 0.6.2中被废弃，
    并且从Solidity 0.7.0开始不再支持。

具名调用和匿名函数参数
----------------------

函数调用参数可以用名字来表示，如果用 ``{ }`` 括起来的话，
可以用任何顺序，如下面的例子所示。
参数列表在名称上必须与函数声明中的参数列表相一致，但可以有任意的顺序。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract C {
        mapping(uint => uint) data;

        function f() public {
            set({value: 2, key: 3});
        }

        function set(uint key, uint value) public {
            data[key] = value;
        }

    }

省略函数参数名称
-----------------

未使用的参数（尤其是返回参数）的名称可以省略。
这些参数将仍然存在于堆栈中，但它们是不可访问的。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;

    contract C {
        // 省略参数名称
        function func(uint k, uint) public pure returns(uint) {
            return k;
        }
    }


.. index:: ! new, contracts;creating

.. _creating-contracts:

通过 ``new`` 创建合约
========================

一个合约可以使用 ``new`` 关键字创建其他合约。
待创建合约的完整代码必须在创建的合约被编译时知道，
所以递归的创建依赖是不可能的。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    contract D {
        uint public x;
        constructor(uint a) payable {
            x = a;
        }
    }

    contract C {
        D d = new D(4); // 将作为合约 C 构造函数的一部分执行

        function createD(uint arg) public {
            D newD = new D(arg);
            newD.x();
        }

        function createAndEndowD(uint arg, uint amount) public payable {
            // 随合约的创建发送 ether
            D newD = new D{value: amount}(arg);
            newD.x();
        }
    }

正如在例子中所看到的，在使用 ``value`` 选项创建 ``D`` 的实例时，
可以发送以太，但不可能限制气体的数量。
如果创建失败（由于堆栈耗尽，没有足够的余额或其他问题），会抛出一个异常。

加盐合约创建 / create2
-----------------------------------

当创建一个合约时，合约的地址是由创建合约的地址和一个计数器计算出来的，
这个计数器在每次创建合约时都会增加。

如果您指定了选项 ``salt`` （一个32字节的值），
那么合约的创建将使用一种不同的机制来得出新合约的地址。

它将从创建合约的地址、给定的盐值、创建合约的（创建）字节码和构造函数参数中计算出地址。

特别的是，计数器（“nonce”）没有被使用。这使得创建合约时有更多的灵活性。
您能够在新合约创建之前得出它的地址。此外，在创建合约的同时创建其他合约的情况下，
您也可以依赖这个地址。

这里的主要用例是做为链外互动的评判的合约，
只有在有争议的时候才需要创建。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    contract D {
        uint public x;
        constructor(uint a) {
            x = a;
        }
    }

    contract C {
        function createDSalted(bytes32 salt, uint arg) public {
            // 这个复杂的表达式只是告诉您如何预先计算出地址。
            // 它只是用于说明问题。
            // 实际上您只需要 ``new D{salt: salt}(arg)``。
            address predictedAddress = address(uint160(uint(keccak256(abi.encodePacked(
                bytes1(0xff),
                address(this),
                salt,
                keccak256(abi.encodePacked(
                    type(D).creationCode,
                    arg
                ))
            )))));

            D d = new D{salt: salt}(arg);
            require(address(d) == predictedAddress);
        }
    }

.. warning::
    在用加盐方式创建合约时，有一些特殊性。一个合约可以在被销毁后在同一地址重新创建。
    然而，新创建的合约有可能具有不同的部署字节码，
    即使创建字节码是相同的（这是一个要求，否则地址会改变）。
    这是由于构造函数可以查询在两次创建之间可能发生变化的外部状态，
    并在存储之前将其纳入部署字节码。


表达式计算顺序
================

表达式的计算顺序不是特定的（更准确地说，
表达式树中某节点的字节点间的计算顺序不是特定的，但它们的结算肯定会在节点自己的结算之前）。
该规则只能保证语句按顺序执行，并对布尔表达式进行短路处理。

.. index:: ! assignment

赋值
======

.. index:: ! assignment;destructuring

解构赋值和返回多个值
---------------------

Solidity 内部允许元组 (tuple) 类型，也就是一个在编译时元素数量固定的对象列表，
列表中的元素可以是不同类型的对象。这些元组可以用来同时返回多个数值，
也可以用它们来同时赋值给多个新声明的变量或者既存的变量（或通常的 LValues）：

在Solidity中，元组不是适当的类型，它们只能被用来构建表达式的语法分组。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        uint index;

        function f() public pure returns (uint, bool, uint) {
            return (7, true, 2);
        }

        function g() public {
            // 用类型声明的变量，并从返回的元组中分配，
            // 不是所有的元素都必须被指定（但数量必须匹配）。
            (uint x, , uint y) = f();
            // 交换数值的常见技巧 -- 对非数值存储类型不起作用。
            (x, y) = (y, x);
            // 元素可以不使用（也适用于变量声明）。
            (index, , ) = f(); // 将index设置为 7
        }
    }

不可能混合使用声明和非声明变量赋值。
例如，下面的方法是无效的。 ``(x, uint y) = (1, 2);``。

.. note::
    在0.5.0版本之前，给具有更少元素数的元组赋值都是可能的，
    要么在左边填充，要么在右边填充（无论哪个是空的）。
    现在这是不允许的，所以两边必须有相同数量的元素。

.. warning::
    当涉及到引用类型时，在同时向多个变量赋值时要小心，因为这可能导致意外的复制行为。

数组和结构体的复杂情况
----------------------

对于像数组和结构体这样的非值类型，包括 ``bytes`` 和 ``string``，赋值的语义更为复杂，
详见 :ref:`数据位置和赋值行为 <data-location-assignment>`。

在下面的例子中，调用 ``g(x)`` 对 ``x`` 没有影响，
因为它在内存中创建了一个独立的存储值的副本。然而， ``h(x)`` 成功地修改了 ``x``，
因为传递了一个引用而不是一个拷贝。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;

    contract C {
        uint[20] x;

        function f() public {
            g(x);
            h(x);
        }

        function g(uint[20] memory y) internal pure {
            y[2] = 3;
        }

        function h(uint[20] storage y) internal {
            y[3] = 4;
        }
    }

.. index:: ! scoping, declarations, default value

.. _default-value:

作用域和声明
==============

一个被声明的变量将有一个初始默认值，其字节表示为所有的零。
变量的 "默认值" 是任何类型的典型 "零状态"。
例如， ``bool`` 的默认值是 ``false``。
``uint`` 或 ``int`` 类型的默认值是 ``0``。
对于静态大小的数组和 ``bytes1`` 到 ``bytes32``，
每个单独的元素将被初始化为与其类型相应的默认值。
对于动态大小的数组， ``bytes`` 和 ``string``，默认值是一个空数组或字符串。
对于 ``enum`` 类型，默认值是其第一个成员。

Solidity 中的作用域规则遵循了 C99（与其他很多语言一样）：
变量将会从它们被声明之后可见，直到一对 ``{ }`` 块的结束。
这一规则有个例外，在 for 循环语句中初始化的变量，其可见性仅维持到 for 循环的结束。

类似于参数的变量（函数参数、修改器参数、捕获（catch）参数......）
在后面的代码块中是可见的--对于函数和修改器参数，在函数/修改器的主体中，
对于捕获参数，在捕获块中。

在代码块之外声明的变量，例如函数、合约、用户定义的类型等，
甚至在声明之前就已经可见。
这意味着您可以在声明之前使用状态变量，并递归地调用函数。

因此，下面的例子在编译时不会出现警告，因为这两个变量的名字虽然相同，但作用域不同。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;
    contract C {
        function minimalScoping() pure public {
            {
                uint same;
                same = 1;
            }

            {
                uint same;
                same = 3;
            }
        }
    }

作为 C99 作用域规则的特例，请注意在下边的例子里，
第一次对 ``x`` 的赋值实际上将赋给外层变量而不是内层变量。
在任何情况下，您都会得到一个关于外部变量被影射（译者注：就是说被在内部作用域中由一个同名变量所替代）的警告。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;
    // 这将报告一个警告信息
    contract C {
        function f() pure public returns (uint) {
            uint x = 1;
            {
                x = 2; // this will assign to the outer variable
                uint x;
            }
            return x; // x has value 2
        }
    }

.. warning::
    在0.5.0版本之前，Solidity遵循与JavaScript相同的作用域规则，
    也就是说，在一个函数中的任何地方声明的变量都会在整个函数的作用域中，不管它是在哪里声明。
    下面的例子显示了一个曾经可以编译的代码片段，但从0.5.0版本开始导致了一个错误。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;
    // 这将无法编译
    contract C {
        function f() pure public returns (uint) {
            x = 2;
            uint x;
            return x;
        }
    }


.. index:: ! safe math, safemath, checked, unchecked
.. _unchecked:

检查或不检查的算术
==================

上溢或下溢是指算术运算的结果值，当对一个不受限制的整数执行时，超出了结果类型的范围。

在Solidity 0.8.0之前，算术运算总是在下溢或上溢的情况下被包起来，
这导致广泛使用引入额外检查的库。

从Solidity 0.8.0开始，在默认情况下所有的算术运算都会在上溢和下溢时还原，
从而使这些库的使用变得没有必要。

为了获得以前的行为，可以使用一个 ``未检查（unchecked）`` 区块。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.0;
    contract C {
        function f(uint a, uint b) pure public returns (uint) {
            // 这个减法将在下溢时被包起来。
            unchecked { return a - b; }
        }
        function g(uint a, uint b) pure public returns (uint) {
            // 这个减法在下溢时将被还原。
            return a - b;
        }
    }

调用 ``f(2, 3)`` 将返回 ``2**256-1``，而 ``g(2, 3)`` 将导致一个失败的断言。

``unchecked`` 代码块可以在代码块内的任何地方使用，但不能替代代码块。
它也不能被嵌套。

该设置只影响到在语法上位于代码块内的语句。
从 ``unchecked`` 代码块内调用的函数不继承该属性。

.. note::
    为了避免歧义，您不能在一个 ``unchecked`` 代码块内使用 ``_;``。

以下运算符在上溢或下溢时将导致一个失败的断言，
如果在一个未检查的代码块内使用，将被包裹而不会出现错误。

``++``， ``--``， ``+``， 二进制 ``-``， 单进制 ``-``， ``*``， ``/``， ``%``， ``**``

``+=``， ``-=``， ``*=``， ``/=``， ``%=``

.. warning::
    不能使用 ``unchecked`` 代码块来禁止检查除以0或对0取余数。

.. note::
   位操作符不执行上溢或下溢检查。
   这在使用位操作符移位（ ``<<`` ， ``>>``， ``<<=``， ``>>=``）来代替整数除法和2的幂次方时尤其明显。
   例如 ``type(uint256).max << 3`` 不会恢复操作，尽管 ``type(uint256).max * 8`` 会恢复操作。

.. note::
    ``int x = type(int).min; -x;`` 中的第二条语句将导致溢出，
    因为负数范围可以比正数范围多容纳一个值。

明确的类型转换将总是截断，并且永远不会导致失败的断言，但从整数到枚举类型的转换除外。

.. index:: ! exception, ! throw, ! assert, ! require, ! revert, ! errors

.. _assert-and-require:

错误处理：Assert, Require, Revert and Exceptions
======================================================

Solidity 使用状态恢复异常来处理错误。
这种异常将撤消对当前调用（及其所有子调用）中的状态所做的所有更改，
并且还向调用者标记错误。

当异常发生在子调用中时，它们会自动 "冒泡"（也就是说，异常被重新抛出），
除非它们被 ``try/catch`` 语句捕获。这个规则的例外是 ``send``
和低级函数 ``call``， ``delegatecall`` 和 ``staticcall``：
它们在发生异常时返回 ``false`` 作为第一个返回值而不是 "冒泡"。

.. warning::
    如果被调用的账户不存在，低级函数 ``call``， ``delegatecall`` 和 ``staticcall``
    的第一个返回值为 ``true``，这是EVM设计的一部分。
    如果需要的话，必须在调用之前检查账户是否存在。

异常可以包含错误数据，以 :ref:`错误实例 <errors>` 的形式传回给调用者。
内置的错误 ``Error(string)`` 和 ``Panic(uint256)`` 被特殊函数使用，
解释如下。 ``Error`` 用于 "常规" 错误条件，而 ``Panic`` 用于在无错误代码中不应该出现的错误。

通过 ``assert`` 引起Panic异常和通过 ``require`` 引起Error异常
-------------------------------------------------------------

快捷函数 ``assert`` 和 ``require`` 可以用来检查条件，如果不符合条件就抛出一个异常。

``assert`` 函数创建了一个 ``Panic(uint256)`` 类型的错误。
在某些情况下，编译器也会产生同样的错误，如下所述。

Assert应该只用于测试内部错误，以及检查不变量。
正确运行的代码不应该创建一个Panic异常，甚至在无效的外部输入时也不应该。
如果发生这种情况，那么您的合约中就有一个错误，您应该修复它。
语言分析工具可以评估您的合约，以确定会导致Panic异常的条件和函数调用。

在下列情况下会产生一个Panic异常。
与错误数据一起提供的错误代码表明Panic异常的种类。

#. 0x00： 用于一般的编译器插入Panic异常的情况。
#. 0x01： 如果您带参数调用 ``assert`` 时结果是false。
#. 0x11： 如果一个算术运算在一个 ``unchecked { ... }`` 代码块之外导致下溢或上溢。
#. 0x12： 如果您对0做除法或者取余（例如 ``5 / 0`` 或者 ``23 % 0`` ）。
#. 0x21： 如果您把一个太大的或负数的值转换成一个枚举类型。
#. 0x22： 如果您访问一个编码不正确的存储字节数组。
#. 0x31： 如果您在一个空数组上调用 ``.pop()``。
#. 0x32： 如果您访问一个数组， ``bytesN`` 或一个数组切片索引超出数组长度或负索引（即 ``x[i]``，其中 ``i >= x.length`` 或 ``i < 0`` ）。
#. 0x41： 如果您分配了太多的内存空间或创建了一个太大的数组。
#. 0x51： 如果您调用一个零初始化的内部函数类型的变量。

``require`` 函数要么创造一个没有任何数据的错误，
要么创造一个 ``Error(string)`` 类型的错误。
它应该被用来确保在执行之前无法检测到的有效条件。
这包括对输入的条件或调用外部合约的返回值。

.. note::

    目前不能将自定义错误与 ``require`` 结合使用。
    请使用 ``if (!condition) revert CustomError();`` 代替。

在下列情况下，编译器会产生一个 ``Error(string)`` 异常（或者没有数据的异常）。

#. 调用 ``require(x)``，其中 ``x`` 的值为 ``false``。
#. 如果您使用 ``revert()`` 或 ``revert("错误描述")``。
#. 如果您执行一个外部函数调用，目标是一个不包含代码的合约。
#. 如果您的合约通过一个没有 ``payable`` 修饰符的公开函数（包括构造函数和备用函数）接收以太。
#. 如果您的合约通过一个公共的getter函数接收以太。

对于以下情况，来自外部调用的错误数据（如果提供的话）会被转发。
这意味着它既可以引起 `Error` 异常，也可以引起 `Panic` 异常（或提供的其他什么错误）。

#. 如果 ``.transfer()`` 失败。
#. 如果您通过消息调用一个函数，但它不能正常完成
   （即，耗尽了气体，没有匹配的函数，或自己抛出一个异常），
   除非使用低级操作 ``call``， ``send``， ``delegatecall``， ``callcode``
   或 ``staticcall``。低级操作从不抛出异常，但通过返回 ``false`` 表示失败。
#. 如果您使用 ``new`` 关键字创建一个合约，
   但合约创建 :ref:`没有正常完成 <creating-contracts>`。

您可以选择为 ``require`` 提供一个信息字符串，但不能为 ``assert`` 提供。

.. note::
    如果您没有给 ``require`` 提供一个字符串参数，它将以空的错误数据进行还原，
    甚至不包括错误选择器。


下面的例子显示了如何使用 ``require`` 来检查输入的条件
和 ``assert`` 进行内部错误检查。

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract Sharer {
        function sendHalf(address payable addr) public payable returns (uint balance) {
            require(msg.value % 2 == 0, "Even value required.");
            uint balanceBeforeTransfer = address(this).balance;
            addr.transfer(msg.value / 2);
            // 由于转账失败后抛出异常并且不能在这里回调，
            // 因此我们应该没有办法仍然有一半的钱。
            assert(address(this).balance == balanceBeforeTransfer - msg.value / 2);
            return address(this).balance;
        }
    }

在内部， Solidity 会执行恢复操作（指令 ``0xfd`` ）。
这会导致 EVM 恢复对状态所做的所有更改。恢复的原因是不能继续安全地执行，
因为没有实现预期的效果，还因为我们想保留交易的原子性，
所以最安全的做法是恢复所有更改并使整个交易（或至少是调用）不产生效果。

在这两种情况下，调用者可以使用 ``try``/ ``catch`` 对这种失败做出处理，
但被调用者的变化将总是被恢复。

.. note::

    在Solidity 0.8.0之前，Panic异常曾使用 ``invalid`` 操作码，
    它消耗了所有可用于调用的气体。在Metropolis发布之前，
    使用 ``require`` 的异常会消耗所有气体。

.. _revert-statement:

``revert``
----------

可以使用 ``revert`` 语句和 ``revert`` 函数来触发直接恢复。

``revert`` 语句将一个自定义的错误作为直接参数，没有括号：

    revert CustomError(arg1, arg2);

出于向后兼容的原因，还有一个 ``revert()`` 函数，
它使用圆括号并接受一个字符串：

    revert();
    revert("description");

错误数据将被传回给调用者，可以在那里捕获。
使用 ``revert()`` 会导致没有任何错误数据的还原，
而 ``revert("description")`` 将创建一个 ``Error(string)`` 错误。

使用一个自定义的错误实例通常会比字符串描述便宜得多，
因为您可以使用错误的名称来描述它，它的编码只有四个字节。
可以通过NatSpec提供更长的描述，这不会产生任何费用。

下面的例子显示了如何将一个错误字符串和一个自定义的错误实例
与 ``revert`` 和相应的 ``require`` 一起使用。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    contract VendingMachine {
        address owner;
        error Unauthorized();
        function buy(uint amount) public payable {
            if (amount > msg.value / 2 ether)
                revert("Not enough Ether provided.");
            // 另一种方法：
            require(
                amount <= msg.value / 2 ether,
                "Not enough Ether provided."
            );
            // 执行购买。
        }
        function withdraw() public {
            if (msg.sender != owner)
                revert Unauthorized();

            payable(msg.sender).transfer(address(this).balance);
        }
    }

``if (!condition) revert(...);`` 和 ``require(condition, ...);`` 这两种方式是等价的，
只要 ``revert`` 和 ``require`` 的参数没有副作用，比如说它们只是字符串。

.. note::
    ``require`` 函数和其他函数一样。这意味着在执行函数本身之前，所有参数都会被评估。
    特别是，在 ``require(condition, f())`` 中，即使 ``condition`` 为真，
    函数 ``f`` 也被执行。

提供的字符串是 :ref:`ABI编码 <ABI>` 之后的，就像调用一个函数 ``Error(string)`` 一样。
在上面的例子中， ``revert("Not enough Ether provided.");`` 返回以下十六进制作为错误返回数据：

.. code::

    0x08c379a0                                                         // Error(string) 的函数选择器
    0x0000000000000000000000000000000000000000000000000000000000000020 // 数据的偏移量（32）
    0x000000000000000000000000000000000000000000000000000000000000001a // 字符串长度（26）
    0x4e6f7420656e6f7567682045746865722070726f76696465642e000000000000 // 字符串数据（"Not enough Ether provided." 的 ASCII 编码，26字节）

调用者可以使用 ``try`` / ``catch`` 检索所提供的消息，如下所示。

.. note::
    以前有一个叫 ``throw`` 的关键字，其语义与 ``revert()`` 相同，
    在0.4.13版本中被弃用，在0.5.0版本中被删除。


.. _try-catch:

``try`` / ``catch``
---------------------

外部调用的失败可以用 try/catch 语句来捕获，如下所示：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.1;

    interface DataFeed { function getData(address token) external returns (uint value); }

    contract FeedConsumer {
        DataFeed feed;
        uint errorCount;
        function rate(address token) public returns (uint value, bool success) {
            // 如果有10个以上的错误，就永久停用该机制。
            require(errorCount < 10);
            try feed.getData(token) returns (uint v) {
                return (v, true);
            } catch Error(string memory /*reason*/) {
                // 如果在getData中调用revert，
                // 并且提供了一个原因字符串，
                // 则执行该命令。
                errorCount++;
                return (0, false);
            } catch Panic(uint /*errorCode*/) {
                // 在发生Panic异常的情况下执行，
                // 即出现严重的错误，如除以零或溢出。
                // 错误代码可以用来确定错误的种类。
                errorCount++;
                return (0, false);
            } catch (bytes memory /*lowLevelData*/) {
                // 在使用revert()的情况下，会执行这个命令。
                errorCount++;
                return (0, false);
            }
        }
    }

``try`` 关键字后面必须有一个表达式，代表外部函数调用或合约建（ ``new ContractName()`` ）。
表达式中的错误不会被捕获（例如，如果它是一个复杂的表达式，也涉及到内部函数调用），
只有外部调用本身发生恢复。
接下来的 ``returns`` 部分（是可选的）声明了与外部调用返回的类型相匹配的返回变量。
如果没有错误，这些变量将被分配，合约执行将在第一个成功代码块内继续。
如果到达成功代码块的末端，则在 ``catch`` 块之后继续执行。

Solidity 根据错误的类型，支持不同种类的捕获块：

- ``catch Error(string memory reason) { ... }``： 这个catch子句会被执行，
  如果错误是由 ``revert("reasonString")`` 或 ``require(false, "reasonString")`` 造成的
  （或内部错误造成的）。

- ``catch Panic(uint errorCode) { ... }``： 如果错误是由Panic异常引起的，
  例如由失败的 ``assert``、除以0、无效的数组访问、算术溢出和其他原因引起的，这个catch子句将被运行。

- ``catch (bytes memory lowLevelData) { ... }``： 如果错误签名与其他子句不匹配，
  或者在解码错误信息时出现了错误，或者没有与异常一起提供错误数据，
  那么这个子句就会被执行。在这种情况下，声明的变量提供了对低级错误数据的访问。

- ``catch { ... }``： 如果您对错误数据不感兴趣，您可以直接使用
  ``catch { ... }`` （甚至作为唯一的catch子句）来代替前面的子句。


计划在未来支持其他类型的错误数据。字符串 ``Error`` 和 ``Panic`` 目前是按原样解析的，不作为标识符处理。

为了捕捉所有的错误情况，您至少要有 ``catch { ...}`` 或 ``catch (bytes memory lowLevelData) { ... }`` 子句。

在 ``returns`` 和 ``catch`` 子句中声明的变量只在后面的代码块中有作用域。

.. note::

    如果在 try/catch 语句内部的返回数据解码过程中发生错误，
    这将导致当前执行的合约出现异常，正因为如此，它不会在catch子句中被捕获。
    如果在 ``catch Error(string memory reason)`` 的解码过程中出现错误，
    并且有一个低级的catch子句，那么这个错误就会在那里被捕获。

.. note::

    如果执行到一个catch代码块，那么外部调用的状态改变效果已经被恢复。
    如果执行到了成功代码块，那么这些影响就没有被还原。
    如果影响已经被还原，那么执行要么在catch代码块中继续，
    要么try/catch语句的执行本身被还原（例如由于上面提到的解码失败或者由于没有提供低级别的catch子句）。

.. note::
    调用失败背后的原因可能是多方面的。不要认为错误信息是直接来自被调用的合约：
    错误可能发生在调用链的更深处，被调用的合约只是转发了它。
    另外，这可能是由于消耗完气体值的情况，而不是故意的错误状况。
    调用方总是保留调用中至少1/64的气体值，
    因此，即使被调用合约没有气体了，调用方仍然有一些气体。
