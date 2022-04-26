.. index:: ! visibility, external, public, private, internal

.. |visibility-caveat| replace:: 标记一些变量为 ``private`` 或 ``internal``，只能防止其他合约读取或修改信息，但它仍然会被区块链之外的整个世界看到。

.. _visibility-and-getters:

**********************
可见性和 getter 函数
**********************

状态变量的可见性
=================

``public``
    公开状态变量与内部变量的不同之处在于，编译器会自动为它们生成 :ref:`getter函数 <getter-functions>`，
    从而允许其他合约读取它们的值。当在同一个合约中使用时，外部访问（例如 ``this.x``）会调用getter，
    而内部访问（例如 ``x``）会直接从存储中获取变量值。
    Setter函数没有被生成，所以其他合约不能直接修改其值。

``internal``
    内部状态变量只能从它们所定义的合约中和派生合约中访问。
    它们不能被外部访问。
    这是状态变量的默认可见性。

``private``
    私有状态变量就像内部变量一样，但它们在派生合约中是不可见的。

.. 警告::
    |visibility-caveat|

函数的可见性
===================

Solidity 有两种函数调用：确实创建了实际 EVM 消息调用的外部函数和不创建 EVM 消息调用的内部函数。
此外，派生合约可能无法访问内部函数。
这就产生了四种类型的函数的可见性。

``external``
    外部函数作为合约接口的一部分，意味着我们可以从其他合约和交易中调用。
    一个外部函数 ``f`` 不能从内部调用
    （即 ``f()`` 不起作用，但 ``this.f()`` 可以）。

``public``
    公开函数是合约接口的一部分，可以在内部或通过消息调用。

``internal``
    内部函数只能从当前的合约或从它派生出来的合约中访问。
    它们不能被外部访问。
    由于它们没有通过合约的ABI暴露在外部，它们可以接受内部类型的参数，如映射或存储引用。

``private``
    私有函数和内部函数一样，但它们在派生合约中是不可见的。

.. 警告::
    |visibility-caveat|

在状态变量的类型之后，以及在函数的参数列表和返回参数列表之间，都会给出可见性指定符。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f(uint a) private pure returns (uint b) { return a + 1; }
        function setData(uint a) internal { data = a; }
        uint public data;
    }

在下面的例子中，合约 ``D``, 可以调用 ``c.getData()`` 来检索状态存储中 ``data`` 的值，
但不能调用 ``f``。 合约 ``E`` 是从合约 ``C`` 派生出来的，因此可以调用 ``compute``。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        uint private data;

        function f(uint a) private pure returns(uint b) { return a + 1; }
        function setData(uint a) public { data = a; }
        function getData() public view returns(uint) { return data; }
        function compute(uint a, uint b) internal pure returns (uint) { return a + b; }
    }

    // This will not compile
    contract D {
        function readData() public {
            C c = new C();
            uint local = c.f(7); // 错误：成员 `f` 不可见
            c.setData(3);
            local = c.getData();
            local = c.compute(3, 5); // 错误：成员 `compute` 不可见
        }
    }

    contract E is C {
        function g() public {
            C c = new C();
            uint val = compute(3, 5); // 访问内部成员（从继承合约访问父合约成员）
        }
    }

.. index:: ! getter;function, ! function;getter
.. _getter-functions:

Getter 函数
================

编译器会自动为所有 **公开** 状态变量创建getter函数。
对于下面给出的合约，编译器将生成一个名为 ``data`` 的函数，
它没有任何输入参数，并返回一个 ``uint``，
即状态变量 ``data`` 的值。状态变量在声明时可以被初始化。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        uint public data = 42;
    }

    contract Caller {
        C c = new C();
        function f() public view returns (uint) {
            return c.data();
        }
    }

getter函数具有外部可见性。
如果该符号被内部访问（即没有 ``this.``），它被评估为一个状态变量。
如果它被外部访问（即有 ``this.``），它将被评价为一个函数。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract C {
        uint public data;
        function x() public returns (uint) {
            data = 3; // 内部访问
            return this.data(); // 外部访问
        }
    }

如果您有一个数组类型的 ``public`` 状态变量，
那么您只能通过生成的getter函数检索数组的单个元素。
这种机制的存在是为了避免在返回整个数组时产生高额的气体成本。
您可以使用参数来指定要返回的单个元素，例如 ``myArray(0)``。
如果您想在一次调用中返回整个数组，那么您需要写一个函数，例如：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract arrayExample {
        // 公开状态变量
        uint[] public myArray;

        // 编译器生成的getter函数
        /*
        function myArray(uint i) public view returns (uint) {
            return myArray[i];
        }
        */

        // 返回整个数组的函数
        function getArray() public view returns (uint[] memory) {
            return myArray;
        }
    }

现在您可以使用 ``getArray()`` 来检索整个数组，
而不是使用 ``myArray(i)``，它每次调用只返回一个元素。

下一个例子稍微复杂一些：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract Complex {
        struct Data {
            uint a;
            bytes3 b;
            mapping (uint => uint) map;
            uint[3] c;
            uint[] d;
            bytes e;
        }
        mapping (uint => mapping(bool => Data[])) public data;
    }

它生成了一个如下形式的函数。结构中的映射和数组（字节数组除外）被省略了，
因为没有好的方法来选择单个结构成员或为映射提供一个键：

.. code-block:: solidity

    function data(uint arg1, bool arg2, uint arg3)
        public
        returns (uint a, bytes3 b, bytes memory e)
    {
        a = data[arg1][arg2][arg3].a;
        b = data[arg1][arg2][arg3].b;
        e = data[arg1][arg2][arg3].e;
    }
