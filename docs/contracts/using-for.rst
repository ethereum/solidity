.. index:: ! using for, library

.. _using-for:

*********
Using For
*********

指令 ``use A for B;`` 可以用来将库函数（来自库 ``A``）附加到合约背景下的任何类型（ ``B``）。
这些函数将接收它们被调用的对象作为其第一个参数（就像Python中的 ``self`` 变量）。

``using A for *;`` 的效果是，库合约 ``A`` 中的函数被附加在 *任意* 的类型上。

在这两种情况下， *所有* 函数都会被附加一个参数，
即使它们的第一个参数类型与对象的类型不匹配。 函数调用和重载解析时才会做类型检查。

``using A for B;`` 指令只在当前的合约内有效，包括其所有的功能，在使用该指令的合约之外没有效果。
该指令只能在合约内使用，不能在其任何函数内使用。

让我们用这种方式将 :ref:`库合约` 中的 set 例子重写:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;


    // 这是和之前一样的代码，只是没有注释。
    struct Data { mapping(uint => bool) flags; }

    library Set {
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
        using Set for Data; // 这里是关键的修改
        Data knownValues;

        function register(uint value) public {
            // 这里， Data 类型的所有变量都有与之相对应的成员函数。
            // 下面的函数调用和 `Set.insert(knownValues, value)` 的效果完全相同。
            require(knownValues.insert(value));
        }
    }

也可以像这样扩展基本类型:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.8 <0.9.0;

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

    contract C {
        using Search for uint[];
        uint[] data;

        function append(uint value) public {
            data.push(value);
        }

        function replace(uint _old, uint _new) public {
            // 执行库函数调用
            uint index = data.indexOf(_old);
            if (index == type(uint).max)
                data.push(_new);
            else
                data[index] = _new;
        }
    }

注意，所有的外部库调用实际都是EVM函数调用。
这意味着，如果你传递内存或值类型，将进行拷贝，即使是在 ``self`` 变量的情况下。
唯一不进行拷贝的情况是当使用存储引用变量或调用内部库函数时。
