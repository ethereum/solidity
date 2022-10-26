.. index:: ! library, callcode, delegatecall

.. _libraries:

*********
库合约
*********

库合约与普通合约类似，但是它们只需要在特定的地址部署一次，
并且它们的代码可以通过 EVM 的 ``DELEGATECALL`` (Homestead 之前使用 ``CALLCODE`` 关键字)特性进行重用。
这意味着如果库函数被调用，它的代码在调用合约的上下文中执行，
即 ``this`` 指向调用合约，特别是可以访问调用合约的存储。
因为每个库合约都是一段独立的代码，所以它仅能访问调用合约明确提供的状态变量（否则它就无法通过名字访问这些变量）。
如果库函数不修改状态（也就是说，如果它们是 ``view`` 或者 ``pure`` 函数），
它们可以通过直接调用来使用（即不使用 ``DELEGATECALL`` 关键字），
这是因为我们假定库合约是无状态的。
特别的是，销毁一个库合约是不可能的。

.. note::
    在0.4.20版本之前，有可能通过规避Solidity的类型系统来破坏库合约。
    从该版本开始，库合约包含一个 :ref:`保护机制 <call-protection>`，
    不允许直接调用修改状态的函数（即没有 ``DELEGATECALL`` ）。

库合约可以看作是使用他们的合约的隐式的基类合约。
虽然它们在继承关系中不会显式可见，但调用库函数与调用显式的基类合约十分类似
（如果 ``L`` 是库合约的话，可以使用 ``L.f()`` 调用库函数）。
当然，需要使用内部调用约定来调用内部函数，这意味着所有的内部类型都可以被传递，
类型 :ref:`存储在内存 <data-location>` 将被引用传递而不是复制。
为了在EVM中实现这一点，从合约中调用的内部库函数的代码和其中调用的所有函数将在编译时包含在调用合约中，
并使用常规的 ``JUMP`` 调用，而不是 ``DELEGATECALL``。

.. note::
    当涉及到公共函数时，继承的类比就失效了。
    用 ``L.f()`` 调用公共库函数的结果是一个外部调用（准确地说，是 ``DELEGATECALL`` ）。
    相反，当 ``A.f()`` 是当前合约的基类合约时， ``A.f()`` 是一个内部调用。

.. index:: using for, set

下面的示例说明如何使用库（但也请务必看看 :ref:`using for <using-for>` 有一个实现 set 更好的例子）。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;


    // 我们定义了一个新的结构体数据类型，用于在调用合约中保存数据。
    struct Data {
        mapping(uint => bool) flags;
    }

    library Set {
        // 注意第一个参数是 “storage reference”类型，
        // 因此在调用中参数传递的只是它的存储地址而不是内容。
        // 这是库函数的一个特性。如果该函数可以被视为对象的方法，
        // 则习惯称第一个参数为 `self` 。
        function insert(Data storage self, uint value)
            public
            returns (bool)
        {
            if (self.flags[value])
                return false; // 已经存在
            self.flags[value] = true;
            return true;
        }

        function remove(Data storage self, uint value)
            public
            returns (bool)
        {
            if (!self.flags[value])
                return false; // 不存在
            self.flags[value] = false;
            return true;
        }

        function contains(Data storage self, uint value)
            public
            view
            returns (bool)
        {
            return self.flags[value];
        }
    }


    contract C {
        Data knownValues;

        function register(uint value) public {
            // 不需要库的特定实例就可以调用库函数，
            // 因为当前合约就是 “instance”。
            require(Set.insert(knownValues, value));
        }
        // 如果我们愿意，我们也可以在这个合约中直接访问 knownValues.flags。
    }

当然，您不必按照这种方式去使用库：它们也可以在不定义结构数据类型的情况下使用。
函数也不需要任何存储引用参数，库可以出现在任何位置并且可以有多个存储引用参数。

调用 ``Set.contains``， ``Set.insert`` 和 ``Set.remove`` 都被编译为对外部合约/库的调用（ ``DELEGATECALL`` ）。
如果使用库，请注意实际执行的是外部函数调用。
``msg.sender``， ``msg.value`` 和 ``this`` 在调用中将保留它们的值，
（在 Homestead 之前，因为使用了 ``CALLCODE`` ，改变了 ``msg.sender`` 和 ``msg.value``)。

下面的例子显示了如何使用 :ref:`存储在内存中的类型 <data-location>` 和库合约中的内部函数，
以实现自定义类型，而没有外部函数调用的开销：

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.0;

    struct bigint {
        uint[] limbs;
    }

    library BigInt {
        function fromUint(uint x) internal pure returns (bigint memory r) {
            r.limbs = new uint[](1);
            r.limbs[0] = x;
        }

        function add(bigint memory a, bigint memory b) internal pure returns (bigint memory r) {
            r.limbs = new uint[](max(a.limbs.length, b.limbs.length));
            uint carry = 0;
            for (uint i = 0; i < r.limbs.length; ++i) {
                uint limbA = limb(a, i);
                uint limbB = limb(b, i);
                unchecked {
                    r.limbs[i] = limbA + limbB + carry;

                    if (limbA + limbB < limbA || (limbA + limbB == type(uint).max && carry > 0))
                        carry = 1;
                    else
                        carry = 0;
                }
            }
            if (carry > 0) {
                // 太差了，我们需要增加一个 limb
                uint[] memory newLimbs = new uint[](r.limbs.length + 1);
                uint i;
                for (i = 0; i < r.limbs.length; ++i)
                    newLimbs[i] = r.limbs[i];
                newLimbs[i] = carry;
                r.limbs = newLimbs;
            }
        }

        function limb(bigint memory a, uint index) internal pure returns (uint) {
            return index < a.limbs.length ? a.limbs[index] : 0;
        }

        function max(uint a, uint b) private pure returns (uint) {
            return a > b ? a : b;
        }
    }

    contract C {
        using BigInt for bigint;

        function f() public pure {
            bigint memory x = BigInt.fromUint(7);
            bigint memory y = BigInt.fromUint(type(uint).max);
            bigint memory z = x.add(y);
            assert(z.limb(1) > 0);
        }
    }

通过将库合约的类型转换为 ``address`` 类型，即使用 ``address(LibraryName)``，可以获得一个库的地址。

由于编译器不知道库合约的部署地址，
编译后的十六进制代码将包含 ``__$30bbc0abd4d6364515865950d3e0d10953$__`` 形式的占位符。
占位符是完全等同于库合约名的keccak256哈希值的34个字符的前缀，例如 ``libraries/bigint.sol:BigInt``，
如果该库存储在 ``libraries/`` 目录下一个名为 ``bigint.sol`` 的文件中。
这样的字节码是不完整的，不应该被部署。占位符需要被替换成实际地址。
您可以在编译库的时候把它们传递给编译器，或者用链接器来更新已经编译好的二进制文件。
参见 :ref:`library-linking`，了解如何使用命令行编译器进行链接。

与合约相比，库在以下方面受到限制：

- 它们不能有状态变量
- 它们不能继承，也不能被继承
- 它们不能接收以太
- 它们不能被销毁

(这些可能会在以后的时间里被解除)。

.. _library-selectors:
.. index:: ! selector; of a library function

库合约中的函数签名和选择器
==============================================

虽然对公共或外部库函数的外部调用是可能的，但这种调用的调用惯例被认为是 Solidity 内部的，
与常规 :ref:`合约 ABI <ABI>` 所指定的不一样。
外部库函数比外部合约函数支持更多的参数类型，例如递归结构和存储指针。
由于这个原因，用于计算4字节选择器的函数签名是按照内部命名模式计算的，
合约ABI中不支持的类型的参数使用内部编码。

签名中的类型使用了以下标识符：

- 值类型、非存储的 ``string`` 和非存储的 ``bytes`` 使用与合约ABI中相同的标识符。
- 非存储数组类型遵循与合约ABI中相同的惯例，即 ``<type>[]`` 用于动态数组，
  ``<type>[M]`` 用于 ``M`` 元素的固定大小数组。
- 非存储结构体用其完全等同于的名称来指代，即 ``C.S`` 代表 ``contract C { struct S { ... } }``。
- 存储指针映射使用 ``mapping(<keyType> => <valueType>) storage``，
  其中 ``<keyType>`` 和 ``<valueType>`` 分别是映射的键和值类型的标识。
- 其他存储指针类型使用其对应的非存储类型的类型标识符，但在其后面附加一个空格，即 ``storage``。

参数的编码与普通合约ABI相同，除了存储指针，
它被编码为一个 ``uint256`` 值，指的是它们所指向的存储槽。

与合约ABI类似，选择器由签名的Keccak256-hash的前四个字节组成。
它的值可以通过使用 ``.selector`` 成员从 Solidity 获得，如下：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.14 <0.9.0;

    library L {
        function f(uint256) external {}
    }

    contract C {
        function g() public pure returns (bytes4) {
            return L.f.selector;
        }
    }



.. _call-protection:

库的调用保护
=============================

正如介绍中提到的那样，如果库的代码是通过 ``CALL`` 来执行，
而不是 ``DELEGATECALL`` 或者 ``CALLCODE``，
那么执行的结果会被恢复， 除非是对 ``view`` 或者 ``pure`` 函数的调用。

EVM没有提供一个直接的方法让合约检测它是否被使用 ``CALL`` 调用，
但是合约可以使用 ``ADDRESS`` 操作码来找出它当前运行的 "位置"。
生成的代码将这个地址与构造时使用的地址进行比较，以确定调用的模式。

更具体地说，一个库合约的运行时代码总是以 push 指令开始，
在编译时它是一个20字节的零。
当部署代码运行时，这个常数在内存中被当前地址所取代，这个修改后的代码被存储在合约中。
在运行时，这导致部署时的地址成为第一个被推入堆栈的常数，
对于任何 非-view 和 非-pure 函数，调度器代码会将当前地址与这个常数进行比较。

这意味着一个存储在链上的库合约的实际代码，与编译器报告的 ``deployedBytecode`` 的代码不同。
