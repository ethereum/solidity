**********
微支付通道
**********

在这一节中，我们将学习如何建立一个支付通道的实施实例。
它使用加密签名，使以太币在同一当事人之间的重复转移变得安全、即时，并且没有交易费用。
对于这个例子，我们需要了解如何签名和验证签名，并设置支付通道。

创建和验证签名
==============

想象一下，Alice想发送一些以太给Bob，
即Alice是发送方，Bob是接收方。

Alice 只需要在链下发送经过加密签名的信息
(例如通过电子邮件)给Bob，它类似于写支票。

Alice和Bob使用签名来授权交易，这在以太坊的智能合约中是可以实现的。
Alice将建立一个简单的智能合约，让她传输以太币，但她不会自己调用一个函数来启动付款，
而是让Bob来做，从而支付交易费用。

该合约将按以下方式运作：

    1. Alice部署了 ``ReceiverPays`` 合约，附加了足够的以太币来支付将要进行的付款。
    2. Alice通过用她的私钥签署一个消息来授权付款。
    3. Alice将经过加密签名的信息发送给Bob。该信息不需要保密（后面会解释），而且发送机制也不重要。
    4. Bob通过向智能合约发送签名的信息来索取他的付款，合约验证了信息的真实性，然后释放资金。

创建签名
--------

Alice不需要与以太坊网络交互来签署交易，这个过程是完全离线的。
在本教程中，我们将使用 `web3.js <https://github.com/ethereum/web3.js>`_ 和
`MetaMask <https://metamask.io>`_ 在浏览器中签署信息。
使用 `EIP-712 <https://github.com/ethereum/EIPs/pull/712>`_ 中描述的方法，
因为它提供了许多其他安全优势。

.. code-block:: javascript

    /// 先进行哈希运算使事情变得更容易
    var hash = web3.utils.sha3("message to sign");
    web3.eth.personal.sign(hash, web3.eth.defaultAccount, function () { console.log("Signed"); });

.. note::
  ``web3.eth.personal.sign`` 把信息的长度加到签名数据中。
  由于我们先进行哈希运算，消息的长度总是正好是32字节，
  因此这个长度前缀总是相同的。

签署内容
------------

对于履行付款的合同，签署的信息必须包括：

    1. 收件人的钱包地址。
    2. 要转移的金额。
    3. 重放攻击的保护。

重放攻击是指一个已签署的信息被重复使用，以获得对第二次交易的授权。
为了避免重放攻击，我们使用与以太坊交易本身相同的技术，
即所谓的nonce，它是一个账户发送的交易数量。
智能合约会检查一个nonce是否被多次使用。

另一种类型的重放攻击可能发生在所有者部署 ``ReceiverPays`` 合约时，
先进行了一些支付，然后销毁该合约。后来，
他们决定再次部署 ``RecipientPays`` 合约，
但新的合约不知道以前合约中使用的nonces，所以攻击者可以再次使用旧的信息。

Alice可以通过在消息中包含合约的地址来防止这种攻击，
并且只有包含合约地址本身的消息才会被接受。
您可以在本节末尾的完整合约的 ``claimPayment()`` 函数的前两行找到这个例子。

组装参数
---------

既然我们已经确定了要在签名信息中包含哪些信息，
我们准备把信息放在一起，进行哈希运算，然后签名。
简单起见，我们把数据连接起来。
`ethereumjs-abi <https://github.com/ethereumjs/ethereumjs-abi>`_ 库提供了一个名为 ``soliditySHA3`` 的函数，
模仿Solidity的 ``keccak256`` 函数应用于使用 ``abi.encodePacked`` 编码的参数的行为。
这里有一个JavaScript函数，为 ``ReceiverPays`` 的例子创建了适当的签名。

.. code-block:: javascript

    // recipient， 是应该被支付的地址。
    // amount，单位是 wei, 指定应该发送多少ether。
    // nonce， 可以是任何唯一的数字，以防止重放攻击。
    // contractAddress， 用于防止跨合约的重放攻击。
    function signPayment(recipient, amount, nonce, contractAddress, callback) {
        var hash = "0x" + abi.soliditySHA3(
            ["address", "uint256", "uint256", "address"],
            [recipient, amount, nonce, contractAddress]
        ).toString("hex");

        web3.eth.personal.sign(hash, web3.eth.defaultAccount, callback);
    }

在Solidity中恢复信息签名者
--------------------------

一般来说，ECDSA的签名由两个参数组成， ``r`` 和 ``s``。
以太坊的签名包括第三个参数 ``v`` ，您可以用它来验证是哪个账户的私钥被用来签署信息，
以及作为交易的发送者。Solidity 提供了一个内置函数
:ref:`ecrecover <mathematical-and-cryptographic-functions>`，
它接受一个消息以及 ``r``, ``s`` 和 ``v`` 参数，然后返回用于签署该消息的地址。

提取签名参数
------------

web3.js 产生的签名是 ``r``, ``s`` 和 ``v`` 的拼接的，
所以第一步是把这些参数分开。您可以在客户端这样做，
但在智能合约内这样做意味着你只需要发送一个签名参数而不是三个。
将一个字节数组分割成它的组成部分是很麻烦的，
所以我们在 ``splitSignature`` 函数中使用
:doc:`inline assembly <assembly>` 完成这项工作（本节末尾的完整合约中的第三个函数）。


计算信息哈希值
--------------

智能合约需要确切地知道哪些参数用于签名，
因此它必须通过参数重新创建消息，并使用该消息进行签名验证。
在 ``claimPayment`` 函数中，函数 ``prefixed`` 和 ``recoverSigner`` 做了这件事。


完整的合约
----------

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    contract ReceiverPays {
        address owner = msg.sender;

        mapping(uint256 => bool) usedNonces;

        constructor() payable {}

        function claimPayment(uint256 amount, uint256 nonce, bytes memory signature) external {
            require(!usedNonces[nonce]);
            usedNonces[nonce] = true;

            // 这将重新创建在客户端上签名的信息。
            bytes32 message = prefixed(keccak256(abi.encodePacked(msg.sender, amount, nonce, this)));

            require(recoverSigner(message, signature) == owner);

            payable(msg.sender).transfer(amount);
        }

        /// 销毁合约并收回剩余的资金。
        function shutdown() external {
            require(msg.sender == owner);
            selfdestruct(payable(msg.sender));
        }

        /// 签名方法。
        function splitSignature(bytes memory sig)
            internal
            pure
            returns (uint8 v, bytes32 r, bytes32 s)
        {
            require(sig.length == 65);

            assembly {
                // 前32个字节，在长度前缀之后。
                r := mload(add(sig, 32))
                // 第二个32字节。
                s := mload(add(sig, 64))
                // 最后一个字节（下一个32字节的第一个字节）。
                v := byte(0, mload(add(sig, 96)))
            }

            return (v, r, s);
        }

        function recoverSigner(bytes32 message, bytes memory sig)
            internal
            pure
            returns (address)
        {
            (uint8 v, bytes32 r, bytes32 s) = splitSignature(sig);

            return ecrecover(message, v, r, s);
        }

        /// 构建一个前缀哈希值，以模仿 eth_sign 的行为。
        function prefixed(bytes32 hash) internal pure returns (bytes32) {
            return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", hash));
        }
    }


编写一个简单的支付通道合约
============================

Alice现在建立了一个简单但完整的支付通道的实现。
支付通道使用加密签名来安全、即时地重复转移以太币，
并且没有交易费用。

什么是支付通道？
----------------

支付通道允许参与者在不使用交易的情况下重复转移以太币。
这意味着，你可以避免与交易相关的延迟和费用。
我们将探讨两方（Alice和Bob）之间一个简单的单向支付通道。它涉及三个步骤：

    1. Alice用以太币为智能合约提供资金。这就 "打开" 了支付通道。
    2. Alice签署信息，说明欠接收者多少以太币。这个步骤对每一笔付款都要重复进行。
    3. Bob "关闭" 支付通道，取出他的那部分以太币，并将剩余部分发回给发送方。

.. note::
  只有步骤1和3需要以太坊交易，意味着步骤2中发送方可以通过链下方法（如电子邮件）
  向接收方发送加密签名的信息。这意味着只需要两个交易就可以支持任何数量的转移。

Bob保证会收到他的资金，因为智能合约托管了以太币，
并兑现了一个有效的签名信息。智能合约也强制执行超时，
所以即使接收者拒绝关闭通道，Alice也能保证最终收回她的资金。
由支付通道的参与者决定保持通道的开放时间。对于一个短暂的交易，
如向网吧支付每分钟的网络访问费，支付通道可以保持有限的开放时间。
另一方面，对于经常性的支付，如向雇员支付每小时的工资，
支付渠道可能会保持开放几个月或几年。

开通支付渠道
------------

为了开通支付通道，Alice部署了智能合约，
添加了要托管的以太币，并指定了预期接收者和通道存在的最长时间。
这就是本节末尾合同中的函数 ``SimplePaymentChannel``。

进行支付
---------------

Alice通过向Bob发送签名信息进行支付。
这一步骤完全在以太坊网络之外进行。
消息由发送方加密签名，然后直接传送给接收方。

每条信息包括以下信息：

    * 智能合约的地址，用于防止跨合约重放攻击。
    * 到目前为止，欠接收方的以太币的总金额。

一个支付通道只关闭一次，就是在一系列转账结束后。
正因为如此，所发送的签名信息中只有一个能被赎回。
这就是为什么每条签名信息都指定了一个累计的以太币欠款总额，
而不是单个小额支付的金额。接收方自然会选择最新的签名信息来赎回，
因为那是总额最高的签名信息。每个签名信息的nonce不再需要了，
因为智能合约只兑现一个签名信息。
智能合约的地址仍然被用来防止一个支付渠道的签名信息被用于另一个渠道。

下面是经过修改的JavaScript代码，用于对上一节中的信息进行加密签名：


.. code-block:: javascript

    function constructPaymentMessage(contractAddress, amount) {
        return abi.soliditySHA3(
            ["address", "uint256"],
            [contractAddress, amount]
        );
    }

    function signMessage(message, callback) {
        web3.eth.personal.sign(
            "0x" + message.toString("hex"),
            web3.eth.defaultAccount,
            callback
        );
    }

    // contractAddress， 是用来防止跨合同的重放攻击。
    // amount，单位是wei，指定了应该发送多少以太。

    function signPayment(contractAddress, amount, callback) {
        var message = constructPaymentMessage(contractAddress, amount);
        signMessage(message, callback);
    }


关闭支付通道
------------

当Bob准备好接收他的资金时，
是时候通过调用智能合约上的 ``close`` 函数关闭支付通道了。
关闭通道会向接收者支付欠他们的以太币，并销毁合约，
将任何剩余的以太币送回给Alice。
为了关闭通道，Bob需要提供一个由Alice签名的信息。

智能合约必须验证该消息是否包含发送者的有效签名。
进行这种验证的过程与接收者使用签名的过程相同。
Solidity函数 ``isValidSignature`` 和 ``recoverSigner`` 的工作方式
与上一节中的JavaScript对应函数一样，而后者的函数是从 ``ReceiverPays`` 合约中借用的。

只有支付通道的接收者可以调用 ``close`` 函数，
他们自然会传递最新的支付信息，因为该信息带有最高的欠款总额。
如果允许发送者调用这个函数，他们可以提供一个金额较低的签名消息，
骗取接收者的欠款。

该函数会验证签名的信息与给定的参数是否相符。
如果一切正常，接收者就会收到他们的那部分以太币，
而剩下的以太币将通过 ``selfdestruct`` 发送给发送者。
您可以在完整的合约中看到 ``close`` 函数。

通道到期
--------

Bob可以在任何时候关闭支付通道，但如果他们没有这样做，
Alice需要一个方法来收回她的托管资金。在合同部署的时候，设置了一个 *到期时间*。
一旦达到这个时间，Alice可以调用 ``claimTimeout`` 来收回她的资金。
您可以在完整的合约中看到 ``claimTimeout`` 函数。

在这个函数被调用后，Bob不能再接收任何以太。
所以Bob必须在过期前关闭通道，这一点很重要。

完整的合约
-----------------

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    contract SimplePaymentChannel {
        address payable public sender;      // 发送付款的账户。
        address payable public recipient;   // 接收付款的账户。
        uint256 public expiration;  // 超时时间，以防接收者永不关闭支付通道。

        constructor (address payable recipientAddress, uint256 duration)
            payable
        {
            sender = payable(msg.sender);
            recipient = recipientAddress;
            expiration = block.timestamp + duration;
        }

        /// 接收者可以在任何时候通过提供发送者签名的金额来关闭通道，
        /// 接收者将获得该金额，其余部分将返回发送者。
        function close(uint256 amount, bytes memory signature) external {
            require(msg.sender == recipient);
            require(isValidSignature(amount, signature));

            recipient.transfer(amount);
            selfdestruct(sender);
        }

        /// 发送者可以在任何时候延长到期时间。
        function extend(uint256 newExpiration) external {
            require(msg.sender == sender);
            require(newExpiration > expiration);

            expiration = newExpiration;
        }

        /// 如果达到超时时间而接收者没有关闭通道，
        /// 那么以太就会被释放回给发送者。
        function claimTimeout() external {
            require(block.timestamp >= expiration);
            selfdestruct(sender);
        }

        function isValidSignature(uint256 amount, bytes memory signature)
            internal
            view
            returns (bool)
        {
            bytes32 message = prefixed(keccak256(abi.encodePacked(this, amount)));

            // 检查签名是否来自付款方。
            return recoverSigner(message, signature) == sender;
        }

        /// 下面的所有功能是取自 '创建和验证签名' 的章节。

        function splitSignature(bytes memory sig)
            internal
            pure
            returns (uint8 v, bytes32 r, bytes32 s)
        {
            require(sig.length == 65);

            assembly {
                // 前32个字节，在长度前缀之后。
                r := mload(add(sig, 32))
                // 第二个32字节。
                s := mload(add(sig, 64))
                // 最后一个字节（下一个32字节的第一个字节）。
                v := byte(0, mload(add(sig, 96)))
            }

            return (v, r, s);
        }

        function recoverSigner(bytes32 message, bytes memory sig)
            internal
            pure
            returns (address)
        {
            (uint8 v, bytes32 r, bytes32 s) = splitSignature(sig);

            return ecrecover(message, v, r, s);
        }

        /// 构建一个前缀哈希值，以模仿eth_sign的行为。
        function prefixed(bytes32 hash) internal pure returns (bytes32) {
            return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", hash));
        }
    }


.. note::
  函数 ``splitSignature`` 并没有使用所有的安全检查。
  真正的实现应该使用更严格的测试库，例如openzepplin的
  `这个版本 <https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/utils/cryptography/ECDSA.sol>`_
  的这个代码。

验证付款
--------

不同与上一节，支付通道中的信息不会马上被兑换。
接收者会跟踪最新的信息，并在关闭支付通道的时候赎回它。
这意味着接收者对每条信息进行自行验证是至关重要的。
否则就不能保证接收者最终能够得到付款。

接收者应使用以下程序验证每条信息：

    1. 验证签名信息中的合约地址是否与支付通道相符。
    2. 验证新的总额是否为预期的数额。
    3. 确认新的总额不超过代管的以太币数额。
    4. 验证签名是否有效，是否来自于支付通道的发送方。

我们将使用 `ethereumjs-util <https://github.com/ethereumjs/ethereumjs-util>`_
库来编写这个验证。最后一步可以用多种方式完成，我们使用JavaScript。
下面的代码借用了上面 **JavaScript代码** 中加密签名的 ``constructPaymentMessage`` 函数。

.. code-block:: javascript

    // 这模拟了eth_sign 的JSON-RPC构建前缀的方法。
    function prefixed(hash) {
        return ethereumjs.ABI.soliditySHA3(
            ["string", "bytes32"],
            ["\x19Ethereum Signed Message:\n32", hash]
        );
    }

    function recoverSigner(message, signature) {
        var split = ethereumjs.Util.fromRpcSig(signature);
        var publicKey = ethereumjs.Util.ecrecover(message, split.v, split.r, split.s);
        var signer = ethereumjs.Util.pubToAddress(publicKey).toString("hex");
        return signer;
    }

    function isValidSignature(contractAddress, amount, signature, expectedSigner) {
        var message = prefixed(constructPaymentMessage(contractAddress, amount));
        var signer = recoverSigner(message, signature);
        return signer.toLowerCase() ==
            ethereumjs.Util.stripHexPrefix(expectedSigner).toLowerCase();
    }
