.. index:: ! using for, library

.. _using-for:

*********
Using For
*********

指令 ``use A for B;`` 可以用来将函数（ ``A``）作为成员函数附加到任何类型（ ``B``）。
这些函数将接收它们被调用的对象作为其第一个参数（就像Python中的 ``self`` 变量）。

它可以在文件级别或者在合约级别的合约内部有效。

第一部分， ``A``，可以是以下之一：

- 文件级别或库函数的列表（ ``using {f, g, h, L.t} for uint;``）-
  只有这些函数才会被附加到类型上。
- 一个库合约的名字（ ``using L for uint;``）-
  库合约的所有函数（公共函数和内部函数）都被附加到了该类型上。

在文件级别，第二部分， ``B``， 必须是一个显式类型（没有数据位置指定符）。
在合约内部，您还可以使用 ``using L for *;``，
这会使库合约 ``L`` 的所有函数都被附加到 *所有* 类型上。

如果您指定了一个库合约，那么库合约中的 *所有* 函数都会被附加上，
即使那些第一个参数的类型与对象的类型不匹配的函数也是如此。
类型会在函数被调用的时候检查，并执行函数重载解析。

如果您使用函数列表（ ``using {f, g, h, L.t} for uint;``），
那么类型（ ``uint``）必须隐式可转换为这些函数的第一个参数。
即使这些函数都没有被调用，这个检查也会执行。

``using A for B;`` 指令只在当前作用域（合约或当前模块/源单元）内有效，
包括其中所有的函数，在使用它的合约或模块之外没有任何效果。

当在文件级别使用该指令并应用于在同一文件中用户定义类型时，
可以在末尾添加 ``global`` 关键字。
这将产生的效果是，函数将附加到类型的每个地方（包括其他文件），
而不仅仅是在 using 语句的作用域内。

下面我们将使用文件级函数来重写 :ref:`libraries` 部分中的 set 示例。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.13;

    struct Data { mapping(uint => bool) flags; }
    // 现在我们给这个类型附加上函数。
    // 附加的函数可以在模块的其他部分使用。
    // 如果您导入了该模块，
    // 您必须在那里重复using指令，例如
    //   import "flags.sol" as Flags;
    //   using {Flags.insert, Flags.remove, Flags.contains}
    //     for Flags.Data;
    using {insert, remove, contains} for Data;

    function insert(Data storage self, uint value)
        returns (bool)
    {
        if (self.flags[value])
            return false; // already there
        self.flags[value] = true;
        return true;
    }

    function remove(Data storage self, uint value)
        returns (bool)
    {
        if (!self.flags[value])
            return false; // not there
        self.flags[value] = false;
        return true;
    }

    function contains(Data storage self, uint value)
        view
        returns (bool)
    {
        return self.flags[value];
    }


    contract C {
        Data knownValues;

        function register(uint value) public {
            // 这里， Data 类型的所有变量都有与之相对应的成员函数。
            // 下面的函数调用和 `Set.insert(knownValues, value)` 的效果完全相同。
            require(knownValues.insert(value));
        }
    }

也可以通过这种方式来扩展内置类型。
在这个例子中，我们将使用一个库合约。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.13;

    library Search {
        function indexOf(uint[] storage self, uint value)
            public
            view
            returns (uint)
        {
            for (uint i = 0; i < self.length; i++)
                if (self[i] == value) return i;
            return type(uint).max;
        }
    }
    using Search for uint[];

    contract C {
        uint[] data;

        function append(uint value) public {
            data.push(value);
        }

        function replace(uint from, uint to) public {
            // 这将执行库合约中的函数调用
            uint index = data.indexOf(from);
            if (index == type(uint).max)
                data.push(to);
            else
                data[index] = to;
        }
    }

注意，所有的外部库调用实际都是EVM函数调用。
这意味着，如果您传递内存或值类型，将进行拷贝，即使是在 ``self`` 变量的情况下。
唯一不进行拷贝的情况是当使用存储引用变量或调用内部库函数时。
