.. index:: ! event, ! event; anonymous, ! event; indexed, ! event; topic

.. _events:

******
事件
******

Solidity事件在EVM的日志功能之上给出了一个抽象。
应用程序可以通过Ethereum客户端的RPC接口订阅和监听这些事件。

事件是合约的可继承成员。当您调用它们时，
它们会导致参数被存储在交易的日志中--区块链中的一个特殊数据结构。
这些日志与合约的地址相关联，被纳入区块链，
只要有区块可以访问，就会留在那里（目前是永远，但这可能会随着Serenity升级而改变）。
日志及其事件数据不能从合约内部访问（甚至不能从创建它们的合约访问）。

有可能要求为日志提供Merkle证明，
所以如果外部实体向合约提供这样的证明，它可以检查日志是否真的存在于区块链中。
由于合约中仅能访问最近的 256 个区块哈希，所以还需要提供区块头信息。

您可以最多给三个参数添加 ``indexed`` 属性，将它们添加到一个特殊的数据结构中，
称为 :ref:`"topics" <abi_events>`，而不是日志的数据部分。
一个topic只能容纳一个字（32字节），所以如果您为一个索引参数使用 :ref:`引用类型 <reference-types>`，
该值的Keccak-256哈希值将被存储为一个topic中。

所有没有 ``indexed`` 属性的参数都会被 :ref:`ABI 编码 <ABI>` 到日志的数据部分。

Topics允许您用来搜索事件，例如为特定的事件来过滤一系列的区块。
您用来也可以通过发出事件的合约的地址来过滤事件。

例如，下面的代码使用web3.js ``subscribe("logs")``
`方法 <https://web3js.readthedocs.io/en/1.0/web3-eth-subscribe.html#subscribe-logs>`_
来过滤与某一地址值相匹配的日志：

.. code-block:: javascript

    var options = {
        fromBlock: 0,
        address: web3.eth.defaultAccount,
        topics: ["0x0000000000000000000000000000000000000000000000000000000000000000", null, null]
    };
    web3.eth.subscribe('logs', options, function (error, result) {
        if (!error)
            console.log(result);
    })
        .on("data", function (log) {
            console.log(log);
        })
        .on("changed", function (log) {
    });


除非您用 ``anonymous`` 指定符声明事件，否则事件的签名的哈希值是topic之一。
这意味着不可能通过名字来过滤特定的匿名事件，
您只能通过合约地址来过滤。匿名事件的优点是，它们的部署和调用都比较便宜。
它还允许您声明四个索引参数，而不是三个。


.. note::
    由于交易日志只存储事件数据而不存储类型，因此您必须知道事件的类型，
    包括哪个参数被索引以及事件是否是匿名的，以便正确解析数据。
    特别的是，有可能用一个匿名事件 “伪造“ 另一个事件的签名。

.. index:: ! selector; of an event

事件类型的成员方法
===================

- ``event.selector``： 对于非匿名事件，这是一个 ``bytes32`` 值，
  包含事件签名的 ``keccak256`` 哈希值，在默认topic中使用。

示例
=======

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.21 <0.9.0;

    contract ClientReceipt {
        event Deposit(
            address indexed from,
            bytes32 indexed id,
            uint value
        );

        function deposit(bytes32 id) public payable {
            // 事件是用 `emit` 发出的，后面是事件的名称和括号里的参数（如果有）。
            // 任何这样的调用（甚至是深度嵌套）都可以通过过滤 `Deposit`
            // 从JavaScript API中检测出来。
            emit Deposit(msg.sender, id, msg.value);
        }
    }

在JavaScript API中的使用方式如下：

.. code-block:: javascript

    var abi = /* 由编译器产生的abi */;
    var ClientReceipt = web3.eth.contract(abi);
    var clientReceipt = ClientReceipt.at("0x1234...ab67" /* 地址 */);

    var depositEvent = clientReceipt.Deposit();

    // 监视变化
    depositEvent.watch(function(error, result){
        // 结果包含非索引的参数和给 `Deposit` 调用的 topics。
        if (!error)
            console.log(result);
    });


    // 或者通过回调立即开始监视
    var depositEvent = clientReceipt.Deposit(function(error, result) {
        if (!error)
            console.log(result);
    });

上面的输出看起来像下面这样（经过修剪）:

.. code-block:: json

    {
       "returnValues": {
           "from": "0x1111…FFFFCCCC",
           "id": "0x50…sd5adb20",
           "value": "0x420042"
       },
       "raw": {
           "data": "0x7f…91385",
           "topics": ["0xfd4…b4ead7", "0x7f…1a91385"]
       }
    }

了解事件类型的其他资料
======================

- `Javascript 文档 <https://github.com/ethereum/web3.js/blob/1.x/docs/web3-eth-contract.rst#events>`_
- `事件的使用实例 <https://github.com/ethchange/smart-exchange/blob/master/lib/contracts/SmartExchange.sol>`_
- `如何在js中访问它们 <https://github.com/ethchange/smart-exchange/blob/master/lib/exchange_transactions.js>`_
