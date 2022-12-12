.. index:: ! contract;abstract, ! abstract contract

.. _abstract-contract:

******************
抽象合约
******************

当合约中至少有一个函数没有被实现，或者合约没有为其所有的基本合约构造函数提供参数时，
合约必须被标记为 abstract。
即使不是这种情况，合约仍然可以被标记为 abstract，
例如，当您不打算直接创建合约时。
抽象（abstract）合约类似于 :ref:`interfaces`，
但是接口（interface）合约可以声明的内容更加有限。

如下例所示，使用 ``abstract`` 关键字来声明一个抽象合约。
注意，这个合约需要被定义为 abstract，因为函数 ``utterance()`` 被声明了，
但没有提供实现（没有给出实现体 ``{ }``）。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    abstract contract Feline {
        function utterance() public virtual returns (bytes32);
    }

这样的抽象合约不能被直接实例化。如果一个抽象合约本身实现了所有定义的功能，这也是可以的。
抽象合约作为基类的用法在下面的例子中显示：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    abstract contract Feline {
        function utterance() public pure virtual returns (bytes32);
    }

    contract Cat is Feline {
        function utterance() public pure override returns (bytes32) { return "miaow"; }
    }

如果一个合约继承自一个抽象合约，并且没有通过重写实现所有未实现的函数，那么它也需要被标记为抽象的。

注意，没有实现的函数与 :ref:`函数类型 <function_types>` 不同，尽管它们的语法看起来非常相似。

没有实现内容的函数的例子（一个函数声明）：

.. code-block:: solidity

    function foo(address) external returns (address);

类型为函数类型的变量的声明实例：

.. code-block:: solidity

    function(address) external returns (address) foo;

抽象合约将合约的定义与它的实现解耦，提供了更好的可扩展性和自我记录，
促进了像 `模板方法 <https://en.wikipedia.org/wiki/Template_method_pattern>`_ 这样的模式，
并消除了代码的重复。抽象合约的作用与在接口中定义方法的作用相同。
它是抽象合约的设计者说 "我的任何孩子都必须实现这个方法" 的一种方式。


.. note::

  抽象合约不能用一个未实现的virtual函数来重载一个已实现的virtual函数。
