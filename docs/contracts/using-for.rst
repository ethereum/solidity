.. index:: ! using for, library

.. _using-for:

*********
Using For
*********

<<<<<<< HEAD
指令 ``use A for B;`` 可以用来将库函数（来自库 ``A``）附加到合约背景下的任何类型（ ``B``）。
这些函数将接收它们被调用的对象作为其第一个参数（就像Python中的 ``self`` 变量）。

``using A for *;`` 的效果是，库合约 ``A`` 中的函数被附加在 *任意* 的类型上。

在这两种情况下， *所有* 函数都会被附加一个参数，
即使它们的第一个参数类型与对象的类型不匹配。 函数调用和重载解析时才会做类型检查。

``using A for B;`` 指令只在当前的合约内有效，包括其所有的功能，在使用该指令的合约之外没有效果。
该指令只能在合约内使用，不能在其任何函数内使用。

让我们用这种方式将 :ref:`libraries` 中的 set 例子重写:
=======
The directive ``using A for B;`` can be used to attach
functions (``A``) as member functions to any type (``B``).
These functions will receive the object they are called on
as their first parameter (like the ``self`` variable in Python).

It is valid either at file level or inside a contract,
at contract level.

The first part, ``A``, can be one of:

- a list of file-level or library functions (``using {f, g, h, L.t} for uint;``) -
  only those functions will be attached to the type.
- the name of a library (``using L for uint;``) -
  all functions (both public and internal ones) of the library are attached to the type

At file level, the second part, ``B``, has to be an explicit type (without data location specifier).
Inside contracts, you can also use ``using L for *;``,
which has the effect that all functions of the library ``L``
are attached to *all* types.

If you specify a library, *all* functions in the library are attached,
even those where the type of the first parameter does not
match the type of the object. The type is checked at the
point the function is called and function overload
resolution is performed.

If you use a list of functions (``using {f, g, h, L.t} for uint;``),
then the type (``uint``) has to be implicitly convertible to the
first parameter of each of these functions. This check is
performed even if none of these functions are called.

The ``using A for B;`` directive is active only within the current
scope (either the contract or the current module/source unit),
including within all of its functions, and has no effect
outside of the contract or module in which it is used.

When the directive is used at file level and applied to a
user-defined type which was defined at file level in the same file,
the word ``global`` can be added at the end. This will have the
effect that the functions are attached to the type everywhere
the type is available (including other files), not only in the
scope of the using statement.

Let us rewrite the set example from the
:ref:`libraries` section in this way, using file-level functions
instead of library functions.
>>>>>>> 07a7930e73f57ce6ed1c6f0b8dd9aad99e5c3692

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.13;

<<<<<<< HEAD

    // 这是和之前一样的代码，只是没有注释。
=======
>>>>>>> 07a7930e73f57ce6ed1c6f0b8dd9aad99e5c3692
    struct Data { mapping(uint => bool) flags; }
    // Now we attach functions to the type.
    // The attached functions can be used throughout the rest of the module.
    // If you import the module, you have to
    // repeat the using directive there, for example as
    //   import "flags.sol" as Flags;
    //   using {Flags.insert, Flags.remove, Flags.contains}
    //     for Flags.Data;
    using {insert, remove, contains} for Data;

<<<<<<< HEAD
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
=======
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
>>>>>>> 07a7930e73f57ce6ed1c6f0b8dd9aad99e5c3692

    function contains(Data storage self, uint value)
        view
        returns (bool)
    {
        return self.flags[value];
    }


    contract C {
<<<<<<< HEAD
        using Set for Data; // 这里是关键的修改
=======
>>>>>>> 07a7930e73f57ce6ed1c6f0b8dd9aad99e5c3692
        Data knownValues;

        function register(uint value) public {
            // 这里， Data 类型的所有变量都有与之相对应的成员函数。
            // 下面的函数调用和 `Set.insert(knownValues, value)` 的效果完全相同。
            require(knownValues.insert(value));
        }
    }

<<<<<<< HEAD
也可以像这样扩展基本类型:
=======
It is also possible to extend built-in types in that way.
In this example, we will use a library.
>>>>>>> 07a7930e73f57ce6ed1c6f0b8dd9aad99e5c3692

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

<<<<<<< HEAD
        function replace(uint _old, uint _new) public {
            // 执行库函数调用
            uint index = data.indexOf(_old);
=======
        function replace(uint from, uint to) public {
            // This performs the library function call
            uint index = data.indexOf(from);
>>>>>>> 07a7930e73f57ce6ed1c6f0b8dd9aad99e5c3692
            if (index == type(uint).max)
                data.push(to);
            else
                data[index] = to;
        }
    }

注意，所有的外部库调用实际都是EVM函数调用。
这意味着，如果你传递内存或值类型，将进行拷贝，即使是在 ``self`` 变量的情况下。
唯一不进行拷贝的情况是当使用存储引用变量或调用内部库函数时。
