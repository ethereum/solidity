.. index:: contract, state variable, function, event, struct, enum, function;modifier

.. _contract_structure:

*********
合约结构
*********

在 Solidity 中，合约类似于面向对象编程语言中的类。
每个合约中可以包含 :ref:`structure-state-variables`， :ref:`structure-functions`，
:ref:`structure-function-modifiers`， :ref:`structure-events`，
:ref:`structure-errors`， :ref:`structure-struct-types`
和 :ref:`structure-enum-types` 的声明，且合约可以从其他合约继承。

还有一些特殊种类的合同，叫做 :ref:`库合约 <libraries>` 和 :ref:`接口合约 <interfaces>`。

在关于 :ref:`合约 <contracts>` 的部分包含比本节更多的细节，它的作用是提供一个快速的概述。

.. _structure-state-variables:

状态变量
==========

状态变量是指其值被永久地存储在合约存储中的变量。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract SimpleStorage {
        uint storedData; // 状态变量
        // ...
    }

有效的状态变量类型请参阅 :ref:`types` 章节，
对状态变量可见性的可能选择请参阅 :ref:`visibility-and-getters`。

.. _structure-functions:

函数
======

函数是代码的可执行单位。
通常在合约内定义函数，但它们也可以被定义在合约之外。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.1 <0.9.0;

    contract SimpleAuction {
        function bid() public payable { // 函数
            // ...
        }
    }

    // 定义在合约之外的辅助函数
    function helper(uint x) pure returns (uint) {
        return x * 2;
    }

:ref:`function-calls` 可以发生在内部或外部，
并且对其他合约有不同程度的 :ref:`可见性 <visibility-and-getters>`。
:ref:`函数 <functions>` 接受参数并返回变量，以便在它们之间传递参数和值。

.. _structure-function-modifiers:

函数修饰器
===========

函数修饰器可以被用来以声明的方式修改函数的语义(见合约部分的 :ref:`modifiers`)。

重载，也就是具有同一个修饰器的名字但有不同的参数，是不可能的。

与函数一样，修饰器也可以被 :ref:`重载 <modifier-overriding>`。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;

    contract Purchase {
        address public seller;

        modifier onlySeller() { // 修饰器
            require(
                msg.sender == seller,
                "Only seller can call this."
            );
            _;
        }

        function abort() public view onlySeller { // 修饰器的使用
            // ...
        }
    }

.. _structure-events:

事件
======

事件是能方便地调用以太坊虚拟机日志功能的接口。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.21 <0.9.0;

    contract SimpleAuction {
        event HighestBidIncreased(address bidder, uint amount); // 事件

        function bid() public payable {
            // ...
            emit HighestBidIncreased(msg.sender, msg.value); // 触发事件
        }
    }

有关如何声明事件和如何在 dapp 中使用事件的信息，参阅合约章节中的 :ref:`events`。

.. _structure-errors:

错误
======

错误(类型)允许您为失败情况定义描述性的名称和数据。
错误(类型)可以在 :ref:`回滚声明 <revert-statement>` 中使用。
与字符串描述相比，错误(类型)要便宜得多，并允许您对额外的数据进行编码。
您可以使用 NatSpec 格式来向用户描述错误。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    /// 没有足够的资金用于转账。要求 `requested`。
    /// 但只有 `available` 可用。
    error NotEnoughFunds(uint requested, uint available);

    contract Token {
        mapping(address => uint) balances;
        function transfer(address to, uint amount) public {
            uint balance = balances[msg.sender];
            if (balance < amount)
                revert NotEnoughFunds(amount, balance);
            balances[msg.sender] -= amount;
            balances[to] += amount;
            // ...
        }
    }

更多信息请参阅合约章节中的 :ref:`errors`。

.. _structure-struct-types:

结构类型
==========

结构类型是可以将几个变量分组的自定义类型（参阅类型章节中的 :ref:`structs`）。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract Ballot {
        struct Voter { // 结构
            uint weight;
            bool voted;
            address delegate;
            uint vote;
        }
    }

.. _structure-enum-types:

枚举类型
==========

枚举可用来创建由一定数量的'常量值'构成的自定义类型（参阅类型章节中的 :ref:`enums`）。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract Purchase {
        enum State { Created, Locked, Inactive } // 枚举
    }
