*****************
单位和全局变量
*****************

.. index:: wei, finney, szabo, gwei, ether

以太币（Ether） 单位
======================

一个字面常数可以带一个后缀 ``wei``， ``gwei`` 或 ``ether`` 来指定一个以太币的数量，
其中没有后缀的以太数字被认为单位是wei。

.. code-block:: solidity
    :force:

    assert(1 wei == 1);
    assert(1 gwei == 1e9);
    assert(1 ether == 1e18);

单位后缀的唯一作用是乘以10的幂次方。

.. note::
    0.7.0版本中删除了 ``finney`` 和 ``szabo`` 这两个单位。

.. index:: time, seconds, minutes, hours, days, weeks, years

时间单位
==========

诸如 ``seconds``， ``minutes``， ``hours``， ``days`` 和 ``weeks`` 等
后缀在字面常数后面，可以用来指定时间单位，其中秒是基本单位，单位的考虑方式很直白：

* ``1 == 1 seconds``
* ``1 minutes == 60 seconds``
* ``1 hours == 60 minutes``
* ``1 days == 24 hours``
* ``1 weeks == 7 days``

如果您使用这些单位进行日历计算，请注意，由于 `闰秒 <https://en.wikipedia.org/wiki/Leap_second>`_
会造成不是每一年都等于365天，甚至不是每一天都有24小时，而且因为闰秒是无法预测的，
所以需要借助外部的预言机（oracle，是一种链外数据服务，译者注）来对一个确定的日期代码库进行时间矫正。

.. note::
    由于上述原因，在0.5.0版本中删除了后缀 ``years``。

这些后缀单位不能应用于变量。例如，
如果您想用时间单位（例如 days）来将输入变量换算为时间，您可以用以下方式：

.. code-block:: solidity

    function f(uint start, uint daysAfter) public {
        if (block.timestamp >= start + daysAfter * 1 days) {
          // ...
        }
    }

.. _special-variables-functions:

特殊变量和函数
===============

有一些特殊的变量和函数总是存在于全局命名空间，主要用于提供区块链的信息，或者是通用的工具函数。

.. index:: abi, block, coinbase, difficulty, encode, number, block;number, timestamp, block;timestamp, msg, data, gas, sender, value, gas price, origin


区块和交易属性
---------------

- ``blockhash(uint blockNumber) returns (bytes32)``: 当 ``blocknumber`` 是最近的256个区块之一时，给定区块的哈希值；否则返回0。
- ``block.basefee`` （ ``uint``）： 当前区块的基本费用 （ `EIP-3198 <https://eips.ethereum.org/EIPS/eip-3198>`_ 和 `EIP-1559 <https://eips.ethereum.org/EIPS/eip-1559>`_）
- ``block.chainid`` （ ``uint``）： 当前链的ID
- ``block.coinbase`` （ ``address payable``）： 挖出当前区块的矿工地址
- ``block.difficulty`` （ ``uint``）： 挖出当前区块的矿工地址
- ``block.gaslimit`` （ ``uint``）： 当前区块 gas 限额
- ``block.number`` （ ``uint``）： 当前区块号
- ``block.timestamp`` （ ``uint``）： 自 unix epoch 起始到当前区块以秒计的时间戳
- ``gasleft() returns (uint256)``： 剩余的 gas
- ``msg.data`` （ ``bytes calldata``）： 完整的  calldata
- ``msg.sender`` （ ``address``）： 消息发送者（当前调用）
- ``msg.sig`` （ ``bytes4``）： calldata 的前 4 字节（也就是函数标识符）
- ``msg.value`` （ ``uint``）： 随消息发送的 wei 的数量
- ``tx.gasprice`` （ ``uint``）： 随消息发送的 wei 的数量
- ``tx.origin`` （ ``address``）： 交易发起者（完全的调用链）

.. note::
    对于每一个 **外部（external）** 函数调用，
    包括 ``msg.sender`` 和 ``msg.value`` 在内所有 ``msg`` 成员的值都会变化。
    这里包括对库函数的调用。

.. note::
    当合约在链下而不是在区块中包含的交易的背景下计算时，
    您不应该认为 ``block.*`` 和 ``tx.*`` 是指任何特定区块或交易的值。
    这些值是由执行合约的EVM实现提供的，可以是任意的。

.. note::
    不要依赖 ``block.timestamp`` 和 ``blockhash`` 产生随机数，除非您知道自己在做什么。

    时间戳和区块哈希在一定程度上都可能受到挖矿矿工影响。
    例如，挖矿社区中的恶意矿工可以用某个给定的哈希来运行赌场合约的 payout 函数，
    而如果他们没收到钱，还可以用一个不同的哈希重新尝试。

    当前区块的时间戳必须严格大于最后一个区块的时间戳，
    但这里唯一能确保的只是它会是在权威链上的两个连续区块的时间戳之间的数值。

.. note::
    基于可扩展因素，区块哈希不是对所有区块都有效。
    您仅仅可以访问最近 256 个区块的哈希，其余的哈希均为零。

.. note::
    函数 ``blockhash`` 以前被称为 ``block.blockhash``，
    在0.4.22版本中被废弃，在0.5.0版本中被删除。

.. note::
    函数 ``gasleft`` 的前身是 ``msg.gas``，
    在0.4.21版本中被弃用，在0.5.0版本中被删除。

.. note::
    在0.7.0版本中，删除了别名 ``now``（用于 ``block.timestamp``）。

.. index:: abi, encoding, packed

ABI编码和解码函数
-------------------

- ``abi.decode(bytes memory encodedData, (...)) returns (...)``: ABI-解码给定的数据，而类型在括号中作为第二个参数给出。例如： ``(uint a, uint[2] memory b, bytes memory c) = abi.decode(data, (uint, uint[2], bytes))``
- ``abi.encode(...) returns (bytes memory)``： 对给定的参数进行ABI编码
- ``abi.encodePacked(...) returns (bytes memory)``： 对给定参数执行 :ref:`紧打包编码 <abi_packed_mode>`。 请注意，打包编码可能会有歧义!
- ``abi.encodeWithSelector(bytes4 selector, ...) returns (bytes memory)``： ABI-对给定参数进行编码，并以给定的函数选择器作为起始的4字节数据一起返回
- ``abi.encodeWithSignature(string memory signature, ...) returns (bytes memory)``： 相当于 ``abi.encodeWithSelector(bytes4(keccak256(bytes(signature))), ...)``
- ``abi.encodeCall(function functionPointer, (...)) returns (bytes memory)``： 对 ``函数指针`` 的调用进行ABI编码，参数在元组中找到。执行全面的类型检查，确保类型与函数签名相符。结果相当于 ``abi.encodeWithSelector(functionPointer.selector, (...))``。

.. note::
    这些编码函数可用于制作外部函数调用的数据，而无需实际调用外部函数。
    此外， ``keccak256(abi.encodePacked(a, b))`` 是一种计算结构化数据的哈希值的方法
    （但是要注意有可能使用不同的函数参数类型会制作出一个 "哈希碰撞"）。

更多详情请参考 :ref:`ABI <ABI>` 和 :ref:`紧打包编码 <abi_packed_mode>`。

.. index:: bytes members

字节类型的成员
----------------

- ``bytes.concat(...) returns (bytes memory)``: :ref:`将可变数量的字节和byte1, ..., byte32参数串联成一个字节数组 <bytes-concat>`

.. index:: string members

字符串的成员
-----------------

- ``string.concat(...) returns (string memory)``: :ref:`将可变数量的字符串参数串联成一个字符串数组 <string-concat>`


.. index:: assert, revert, require

错误处理
--------------

关于错误处理和何时使用哪个函数的更多细节，
请参见 :ref:`assert 和 require <assert-and-require>` 的专门章节。

``assert(bool condition)``
    如果条件不满足，会导致异常，因此，状态变化会被恢复 - 用于内部错误。

``require(bool condition)``
    如果条件不满足，则恢复状态更改 - 用于输入或外部组件的错误。

``require(bool condition, string memory message)``
    如果条件不满足，则恢复状态更改 - 用于输入或外部组件的错误，可以同时提供一个错误消息。

``revert()``
    终止运行并恢复状态更改。

``revert(string memory reason)``
    终止运行并恢复状态更改，可以同时提供一个解释性的字符串。

.. index:: keccak256, ripemd160, sha256, ecrecover, addmod, mulmod, cryptography,

.. _mathematical-and-cryptographic-functions:

数学和密码学函数
-------------------

``addmod(uint x, uint y, uint k) returns (uint)``
    计算 ``(x + y) % k``，加法会在任意精度下执行，并且加法的结果即使超过 ``2**256`` 也不会被截取。从 0.5.0 版本的编译器开始会加入对 ``k != 0`` 的校验（assert）。

``mulmod(uint x, uint y, uint k) returns (uint)``
    计算 ``(x * y) % k``，乘法会在任意精度下执行，并且乘法的结果即使超过 ``2**256`` 也不会被截取。从 0.5.0 版本的编译器开始会加入对 ``k != 0`` 的校验（assert）。

``keccak256(bytes memory) returns (bytes32)``
    计算输入的 Keccak-256 哈希值。

.. note::

    以前 ``keccak256`` 的别名叫 ``sha3`` ，在0.5.0版本中被删除。

``sha256(bytes memory) returns (bytes32)``
    计算输入的 SHA-256 哈希值。

``ripemd160(bytes memory) returns (bytes20)``
    计算输入的 RIPEMD-160 哈希值。

``ecrecover(bytes32 hash, uint8 v, bytes32 r, bytes32 s) returns (address)``
    利用椭圆曲线签名恢复与公钥相关的地址，错误返回零值。
    函数参数对应于签名的ECDSA值：

    * ``r`` = 签名的前32字节
    * ``s`` = 签名的第二个32字节
    * ``v`` = 签名的最后1个字节

    ``ecrecover`` 返回一个 ``address``，而不是 ``address payable``。
    参见 :ref:`地址类型 <address>` 进行转换，以备您需要向恢复的地址转移资金。

    更多细节，请阅读 `使用示例 <https://ethereum.stackexchange.com/questions/1777/workflow-on-signing-a-string-with-private-key-followed-by-signature-verificatio>`_.

.. warning::

    如果您使用 ``ecrecover``，请注意，一个有效的签名可以变成另一个有效的签名，而不需要知道相应的私钥。
    在Homestead硬分叉中，这个问题对 _transaction_ 签名进行了修复
    （见 `EIP-2 <https://eips.ethereum.org/EIPS/eip-2#specification>`_），
    但ecrecover函数仍然没有改变。

    这通常不是一个问题，除非您要求签名是唯一的，或者用它们来识别个体。
    OpenZeppelin有一个 `ECDSA辅助库 <https://docs.openzeppelin.com/contracts/2.x/api/cryptography#ECDSA>`_，
    您可以用它作为 ``ecrecover`` 的包装，那样就没有这个问题。

.. note::

    当在 *私有区块链* 上运行 ``sha256``， ``ripemd160`` 或 ``ecrecover`` 时，您可能会遇到超出gas（Out-of-Gas）的错误。这是因为这些功能是作为 "预编译合约" 实现的，只有在它们收到第一个消息后才真正存在（尽管它们的合约代码是硬编码的）。向不存在的合约发送消息的成本较高，因此执行时可能会遇到Out-of-Gas错误。这个问题的一个变通方法是，在您的实际合约中使用它们之前，先向每个合约发送Wei（例如1）。这在主网和测试网上都没有问题。

.. index:: balance, codehash, send, transfer, call, callcode, delegatecall, staticcall

.. _address_related:

地址类型的成员
---------------

``<address>.balance`` （ ``uint256`` ）
    以 Wei 为单位的 :ref:`address` 的余额。

``<address>.code`` （ ``bytes memory`` ）
    在 :ref:`address` 的代码（可以是空的）。

``<address>.codehash`` （ ``bytes32`` ）
    :ref:`address` 的代码哈希值

``<address payable>.transfer(uint256 amount)``
    向 :ref:`address` 发送数量为 amount 的 Wei，失败时抛出异常，发送 2300 gas 的矿工费，不可调节。

``<address payable>.send(uint256 amount) returns (bool)``
    向 :ref:`address` 发送数量为 amount 的 Wei，失败时返回 ``false`` 2300 gas 的矿工费用，不可调节。

``<address>.call(bytes memory) returns (bool, bytes memory)``
    用给定的数据发出低级别的 ``CALL``，返回是否成功的结果和数据，发送所有可用 gas，可调节。

``<address>.delegatecall(bytes memory) returns (bool, bytes memory)``
    用给定的数据发出低级别的 ``DELEGATECALL``，返回是否成功的结果和数据，发送所有可用 gas，可调节。

``<address>.staticcall(bytes memory) returns (bool, bytes memory)``
    用给定的数据发出低级别的 ``STATICCALL``，返回是否成功的结果和数据，发送所有可用 gas，可调节。

更多信息，请参见 :ref:`address` 一节。

.. warning::
    您应该尽可能避免在执行另一个合约函数时使用 ``.call()``，因为它绕过了类型检查、函数存在性检查和参数打包。

.. warning::
    使用 ``send`` 有很多危险：如果调用栈深度已经达到 1024（这总是可以由调用者所强制指定），
    转账会失败；并且如果接收者用光了 gas，转账同样会失败。为了保证以太币转账安全，
    总是检查 ``send`` 的返回值，使用 ``transfer`` 或者下面更好的方式： 用接收者提款的模式。

.. warning::
    由于EVM认为对一个不存在的合约的调用总是成功的，
    Solidity在执行外部调用时使用 ``extcodesize`` 操作码进行额外的检查。
    这确保了即将被调用的合约要么实际存在（它包含代码），要么就会产生一个异常。

    对地址而不是合约实例进行低级调用
    （即 ``.call()``, ``.delegatecall()``, ``.staticcall()``, ``.send()`` 和 ``.transfer()``）
    **不包括** 这种检查，这使得它们在gas方面更便宜，但也更不安全。

.. note::
   在0.5.0版本之前，Solidity允许地址成员被合约实例访问，例如 ``this.balance``。
   现在这被禁止了，必须做一个明确的地址转换。 ``address(this).balance``。

.. note::
   如果状态变量是通过低级别的委托调用来访问的，那么两个合约的存储布局必须一致，
   以便被调用的合约能够正确地通过名称来访问调用合约的存储变量。
   当然，如果存储指针作为函数参数被传递的话，情况就不是这样了，就像高层库的情况一样。

.. note::
    在0.5.0版本之前， ``.call``, ``.delegatecall`` 和 ``.staticcall`` 只返回成功状况，
    不返回数据。

.. note::
    在0.5.0版本之前，有一个名为 ``callcode`` 的成员，其语义与 ``delegatecall`` 相似但略有不同。


.. index:: this, selfdestruct

合约相关
----------

``this`` （当前合约类型）
    当前合约，可以明确转换为 :ref:`address`

``selfdestruct(address payable recipient)``
    销毁当前合约，将其资金发送到给定的 :ref:`address` 并结束执行。
    注意， ``selfdestruct`` 有一些从EVM继承的特殊性：

    - 接收合约的接收函数不会被执行。
    - 合约只有在交易结束时才真正被销毁， 任何一个 ``revert`` 可能会 "恢复" 销毁。


此外，当前合约的所有函数都可以直接调用，包括当前函数。

.. note::
    在0.5.0版本之前，有一个叫做 ``suicide`` 的函数，其语义与 ``selfdestruct`` 相同。

.. index:: type, creationCode, runtimeCode

.. _meta-type:

类型信息
----------

表达式 ``type(X)`` 可以用来检索关于 ``X`` 类型的信息。
目前，对这一功能的支持是有限的（ ``X`` 可以是合约类型或整数型），但在未来可能会扩展。

以下是合约类型 ``C`` 的可用属性：

``type(C).name``
    合约的名称。

``type(C).creationCode``
    内存字节数组，包含合约的创建字节码。
    可以在内联程序中用来建立自定义的创建程序，
    特别是通过使用 ``create2`` 操作码。
    这个属性 **不能** 在合约本身或任何派生合约中被访问。
    它会导致字节码被包含在调用站点的字节码中，因此像这样的循环引用是不可能的。

``type(C).runtimeCode``
    内存字节数组，包含合约运行时的字节码。
    通常是由 ``C`` 的构造函数部署的代码。
    如果 ``C`` 有一个使用内联汇编的构造函数，这可能与实际部署的字节码不同。
    还要注意的是，库合约在部署时修改其运行时字节码，以防止常规调用。
    与 ``.creationCode`` 相同的限制也适用于这个属性。

除了上述属性外，以下属性对接口类型 ``I`` 可用：

``type(I).interfaceId``:
    一个 ``bytes4`` 值，是包含给定接口 ``I`` 的 `EIP-165 <https://eips.ethereum.org/EIPS/eip-165>`_ 接口标识符。
    这个标识符被定义为接口本身定义的所有函数选择器的 ``XOR``，不包括所有继承的函数。

以下属性可用于整数类型 ``T``：

``type(T).min``
    类型 ``T`` 所能代表的最小值。

``type(T).max``
    类型 ``T`` 所能代表的最大值。
