.. _inline-assembly:

###############
内联汇编
###############

.. index:: ! assembly, ! asm, ! evmasm


您可以用接近Ethereum虚拟机的语言，将Solidity语句与内联汇编交错使用。
这给了您更精细的控制，这在您通过编写库来增强语言时特别有用。

在 Solidity 中用于内联汇编的语言被称为 :ref:`Yul <yul>`，它在自己的章节中被记录。
本节将只涉及内联汇编代码如何在 Solidity 代码内交互。


.. warning::
    内联汇编是一种在低等级上访问Ethereum虚拟机的方式。
    这绕过了Solidity的几个重要安全功能和检查。
    您应该只在需要它的任务中使用它，而且只有在您对使用它有信心的情况下。


一个内联汇编块由 ``assembly { ... }`` 标记的，其中大括号内的代码是 :ref:`Yul <yul>` 语言中的代码。

内联汇编代码可以访问本地 Solidity 变量，如下所述。

不同的内联汇编块不共享名称空间，
即不能调用或访问一个在不同内联汇编块中定义的Yul函数或变量。

例子
-------

下面例子展示了一个库合约的代码，它可以取得另一个合约的代码，
并将其加载到一个 ``bytes`` 变量中。 通过使用 ``<address>.code``，
这在 "普通Solidity" 中也是可能的。但这里的重点是，可重用的汇编库可以增强 Solidity 语言，
而不需要改变编译器。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    library GetCode {
        function at(address addr) public view returns (bytes memory code) {
            assembly {
                // 获取代码大小，这需要汇编语言
                let size := extcodesize(addr)
                // 分配输出字节数组 – 这也可以不用汇编语言来实现
                // 通过使用 code = new bytes(size)
                code := mload(0x40)
                // 包括补位在内新的 “memory end”
                mstore(0x40, add(code, and(add(add(size, 0x20), 0x1f), not(0x1f))))
                // 把长度保存到内存中
                mstore(code, size)
                // 实际获取代码，这需要汇编语言
                extcodecopy(addr, add(code, 0x20), 0, size)
            }
        }
    }

在优化器不能产生高效代码的情况下，内联汇编也是有益的，例如：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;


    library VectorSum {
        // 这个函数的执行效率比较低，
        // 因为优化器当前无法删除数组访问中的边界检查。
        function sumSolidity(uint[] memory data) public pure returns (uint sum) {
            for (uint i = 0; i < data.length; ++i)
                sum += data[i];
        }

        // 我们知道我们只能在数组范围内访问数组元素，所以我们可以避免检查。
        // 由于 ABI 编码中数组数据的第一个字（32 字节）的位置保存的是数组长度，
        // 所以我们在访问数组元素时需要加入 0x20 作为偏移量。
        function sumAsm(uint[] memory data) public pure returns (uint sum) {
            for (uint i = 0; i < data.length; ++i) {
                assembly {
                    sum := add(sum, mload(add(add(data, 0x20), mul(i, 0x20))))
                }
            }
        }

        // 和上面一样，但在内联汇编内完成整个代码。
        function sumPureAsm(uint[] memory data) public pure returns (uint sum) {
            assembly {
                // 加载数组长度（前 32 字节）
                let len := mload(data)

                // 略过长度字段。
                //
                // 保持临时变量以便它可以在原地增加。
                //
                // 注意：递增data会导致在这个汇编块之后出现一个无法使用的data变量。
                let dataElementLocation := add(data, 0x20)

                // 迭代到数组数据结束。
                for
                    { let end := add(dataElementLocation, mul(len, 0x20)) }
                    lt(dataElementLocation, end)
                    { dataElementLocation := add(dataElementLocation, 0x20) }
                {
                    sum := add(sum, mload(dataElementLocation))
                }
            }
        }
    }

.. index:: selector; of a function

访问外部变量、函数和库
-----------------------------------------------------

您可以通过使用其名称来访问 Solidity 变量和其他标识符。

值类型的局部变量可以直接用于内联汇编。它们既可以被读取也可以被赋值。

指向内存的局部变量是指内存中变量的地址，而不是值本身。
这样的变量也可以被赋值，但请注意，赋值只会改变指针而不是数据，
尊重 Solidity 的内存管理是您的责任。
参见 :ref:`Solidity的惯例 <conventions-in-solidity>`。

同样地，引用静态大小的calldata数组或calldata结构的局部变量会指向calldata中变量的地址，
而不是值本身。变量也可以被分配一个新的偏移量，但是请注意，
没有进行验证以确保变量不会指向超过 ``calldatasize()`` 的地方。

对于外部函数指针，地址和函数选择器可以用 ``x.address`` 和 ``x.selector`` 来访问。
选择器由四个右对齐的字节组成。两个值都可以被赋值。比如说：

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.10 <0.9.0;

    contract C {
        // 将一个新的选择器和地址分配给返回变量 @fun
        function combineToFunctionPointer(address newAddress, uint newSelector) public pure returns (function() external fun) {
            assembly {
                fun.selector := newSelector
                fun.address  := newAddress
            }
        }
    }

对于动态的calldata数组，您可以使用 ``x.offset`` 和 ``x.length`` 访问它们的calldata偏移量（字节）和长度（元素数）。
这两个表达式也可以被赋值，但是和静态情况一样，不会进行验证以确保产生的数据区域在 ``calldatasize()`` 的范围内。


对于本地存储变量或状态变量，一个Yul标识符是不够的，因为它们不一定占据一个完整的存储槽。
因此，它们的 "地址" 是由一个槽和槽内的字节偏移量组成。要检索变量 ``x`` 所指向的槽，
您可以使用 ``x.slot``，要检索字节偏移量，您可以使用 ``x.offset`` 。
使用 ``x`` 本身会导致错误。

您也可以分配给本地存储变量指针的 ``.slot`` 部分。
对于这些（结构、数组或映射）， ``.offset`` 部分总是零。
但不可能分配给状态变量的 ``.slot`` 或 ``.offset`` 部分。

本地 Solidity 变量可用于赋值，例如：

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract C {
        uint b;
        function f(uint x) public view returns (uint r) {
            assembly {
                // 我们忽略了存储槽的偏移量，我们知道在这种特殊情况下它是零。
                r := mul(x, sload(b.slot))
            }
        }
    }

.. warning::
    如果您访问一个跨度小于256位的类型的变量（例如 ``uint64``， ``address``，或 ``bytes16``），
    您不能对不属于该类型的编码的位做任何假设。特别是，不要假设它们是零。
    为了安全起见，在使用前一定要适当清除数据，因为这一点很重要：
    ``uint32 x = f(); assembly { x := and(x, 0xffffff) /* 现在使用 x */ }``
    为了清除有符号的类型，您可以使用 ``signextend`` 操作码。
    ``assembly { signextend(<num_bytes_of_x_minus_one>, x) }``


自Solidity 0.6.0以来，内联汇编变量的名称不能影射内联汇编块范围内可见的任何声明
（包括变量、合约和函数声明）。

自Solidity 0.7.0以来，在内联程序块内声明的变量和函数不能包含 ``.``，
但使用 ``.`` 可以有效地从内联程序块外访问Solidity变量。

需要避免的事情
---------------

内联汇编可能有一个相当高级的外观，但它实际上是非常低级的。
函数调用、循环、if条件和switch条件都可以通过简单的改写规则进行转换，
之后，汇编器为您做的唯一事情就是重新安排函数式的操作码，
为变量访问计算堆栈高度，并在达到汇编局部变量块的末端时移除堆栈槽。

.. _conventions-in-solidity:

Solidity的的惯例
-----------------------

.. _assembly-typed-variables:

类型化变量的值
=========================

与EVM汇编相反，Solidity有比256位更窄的类型，例如： ``uint24``。
为了提高效率，大多数算术运算忽略了类型可以短于256位的事实，高阶位在必要时被清理，
例如，在它们被写入内存前不久或在执行比较之前。
这意味着，如果您从内联汇编中访问这样的变量，您可能不得不先手动清理高阶位。

.. _assembly-memory-management:

内存管理
=================

Solidity以下列方式管理内存。在内存中 ``0x40`` 的位置有一个 "空闲内存指针"。
如果您想分配内存，从这个指针指向的地方开始使用内存，并更新它。
不能保证该内存以前没有被使用过，因此您不能假设其内容为零字节。
没有内置的机制来释放或释放分配的内存。下面是一段汇编代码，
您可以用它来分配内存，它遵循上述的过程：

.. code-block:: yul

    function allocate(length) -> pos {
      pos := mload(0x40)
      mstore(0x40, add(pos, length))
    }

前64字节的内存可以作为短期分配的 "划痕空间（scratch space）"。
空闲内存指针之后的32字节（即从 ``0x60`` 开始）是指永久为零，
并作为空的动态内存数组的初始值使用。
这意味着可分配的内存从 ``0x80`` 开始，也就是空闲内存指针的初始值。

Solidity中内存数组中的元素总是占据32字节的倍数
（对于 ``bytes1[]`` 来说也是如此，但对于 ``bytes`` 和 ``string`` 来说不是这样）。
多维内存数组是指向内存数组的指针。一个动态数组的长度被存储在数组的第一个槽里，后面是数组元素。


.. warning::

    静态大小的内存数组没有长度字段，但以后可能会加入长度字段，
    以便在静态大小的数组和动态大小的数组之间有更好的转换性；所以，不要依赖这个特性。

内存安全
=============

在不使用内联汇编的情况下，编译器可以依靠内存在任何时候都保持一个良好的定义状态。
这对于 :ref:`通过 Yul IR 的新代码生成管道 <ir-breaking-changes>` 来说尤其重要：
这个代码生成路径可以将局部变量从堆栈转移到内存，以避免堆栈过深的错误，并执行额外的内存优化，
如果它可以依赖于对内存使用的某些假设的话。

虽然我们建议始终遵循 Solidity 的内存模型，但内联汇编允许您以不兼容的方式使用内存。
因此，在默认情况下，如果存在任何包含内存操作或在内存中分配给 Solidity 变量的内联汇编块存在的情况，
则禁用将堆栈变量移动到内存和其他内存优化。

然而，您可以特别注释一个汇编块，以表明它实际上是遵循 Solidity 内存模型的，如下所示：

.. code-block:: solidity

    assembly ("memory-safe") {
        ...
    }

特别的是，一个内存安全的汇编块只能访问以下内存范围：

- 通过上述 ``allocate`` 函数这样的机制由自己分配的内存。
- 由 Solidity 分配的内存，例如，在您引用的内存数组的范围内的内存。
- 上面提到的内存偏移量0和64之间的划痕空间。
- 位于汇编块开始时的空闲内存指针值 *之后* 的临时内存，
  即在空闲内存指针上 “分配” 而不更新空闲内存指针的内存。

此外，如果汇编块分配给内存中的 Solidity 变量，则需要确保对 Solidity 变量的访问只访问这些内存范围。

由于这主要是关于优化器的，所以这些限制仍然需要被遵守，即使汇编块回退或终止。
举个例子，下面的汇编片段不是内存安全的，
因为 ``returndatasize()`` 的值可能会超过64字节的划痕空间。

.. code-block:: solidity

    assembly {
      returndatacopy(0, 0, returndatasize())
      revert(0, returndatasize())
    }

另一方面，下面的代码 *是* 内存安全的，
因为超出空闲内存指针所指向的位置的内存可以安全地用作临时划痕空间：

.. code-block:: solidity

    assembly ("memory-safe") {
      let p := mload(0x40)
      returndatacopy(p, 0, returndatasize())
      revert(p, returndatasize())
    }

注意，如果没有后续分配，则不需要更新空闲内存指针，
但只能使用从空闲内存指针给出的当前偏移量开始的内存。

如果内存操作使用的长度为0，那么也可以使用任意偏移量（不仅仅是落在了划痕空间中）；

.. code-block:: solidity

    assembly ("memory-safe") {
      revert(0, 0)
    }

请注意，不仅内联汇编中的内存操作本身可以是内存不安全的，
而且对内存中引用类型的 Solidity 变量的赋值也是如此。例如，以下内容就不是内存安全的：

.. code-block:: solidity

    bytes memory x;
    assembly {
      x := 0x40
    }
    x[0x20] = 0x42;

既不涉及访问内存的任何操作，也不对内存中的任何 Solidity 变量进行赋值的内联汇编，
自动被认为是内存安全的，不需要被注释。

.. warning::
    确保汇编块程序实际满足内存模型是您的责任。
    如果您将一个汇编块注释为内存安全的，但却违反了其中一个内存假设，
    那么这 **将** 导致不正确的和未定义的行为，而这些行为不容易通过测试发现。

如果您正在开发一个要在多个 Solidity 版本之间兼容的库，
您可以使用一个特殊的注释将一个汇编块注释为内存安全的：

.. code-block:: solidity

    /// @solidity memory-safe-assembly
    assembly {
        ...
    }

请注意，我们将在未来的突破性版本中不允许通过注释的方式进行注解；
因此，如果您不关心与旧编译器版本的向后兼容问题，最好使用这种写法的代码字符串形式。
