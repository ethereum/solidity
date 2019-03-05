********************
Micropayment Channel
********************

In this section we will learn how to build an example implementation
of a payment channel. It uses cryptographic signatures to make
repeated transfers of Ether between the same parties secure, instantaneous, and
without transaction fees. For the example, we need to understand how to
sign and verify signatures, and setup the payment channel.

Creating and verifying signatures
=================================

Imagine Alice wants to send a quantity of Ether to Bob, i.e.
Alice is the sender and the Bob is the recipient.

Alice only needs to send cryptographically signed messages off-chain
(e.g. via email) to Bob and it is similar to writing checks.

Alice and Bob use signatures to authorise transactions, which is possible with smart contracts on Ethereum.
Alice will build a simple smart contract that lets her transmit Ether, but instead of calling a function herself
to initiate a payment, she will let Bob do that, and therefore pay the transaction fee.

The contract will work as follows:

    1. Alice deploys the ``ReceiverPays`` contract, attaching enough Ether to cover the payments that will be made.
    2. Alice authorises a payment by signing a message with their private key.
    3. Alice sends the cryptographically signed message to Bob. The message does not need to be kept secret
       (explained later), and the mechanism for sending it does not matter.
    4. Bob claims their payment by presenting the signed message to the smart contract, it verifies the
       authenticity of the message and then releases the funds.

Creating the signature
----------------------

Alice does not need to interact with the Ethereum network to sign the transaction, the process is completely offline.
In this tutorial, we will sign messages in the browser using `web3.js <https://github.com/ethereum/web3.js>`_ and `MetaMask <https://metamask.io>`_, using the method described in `EIP-762 <https://github.com/ethereum/EIPs/pull/712>`_,
as it provides a number of other security benefits.

::
    /// Hashing first makes things easier
    var hash = web3.utils.sha3("message to sign");
    web3.eth.personal.sign(hash, web3.eth.defaultAccount, function () { console.log("Signed"); });

.. note::
  The ``web3.eth.personal.sign`` prepends the length of the message to the signed data. Since we hash first, the message will always be exactly 32 bytes long, and thus this length prefix is always the same.

What to Sign
------------

For a contract that fulfils payments, the signed message must include:

    1. The recipient's address.
    2. The amount to be transferred.
    3. Protection against replay attacks.

A replay attack is when a signed message is reused to claim authorization for
a second action.
To avoid replay attacks we use the same as in Ethereum transactions
themselves, a so-called nonce, which is the number of transactions sent by an
account.
The smart contract checks if a nonce is used multiple times.

Another type of replay attack can occur when the owner deploys a ``ReceiverPays`` smart contract, makes some payments, and then destroys the contract. Later, they decide to deploy the ``RecipientPays`` smart contract again, but the new contract does not know the nonces used in the previous deployment, so the attacker can use the old messages again.

Alice can protect against this attack by including the contract's address in the message, and only messages containing the contract's address itself will be accepted. You can find an example of this in the first two lines of the ``claimPayment()`` function of the full contract at the end of this section.

Packing arguments
-----------------

Now that we have identified what information to include in the signed message,
we are ready to put the message together, hash it, and sign it. For simplicity,
we concatenate the data. The `ethereumjs-abi <https://github.com/ethereumjs/ethereumjs-abi>`_
library provides a function called ``soliditySHA3`` that mimics the behaviour of
Solidity's ``keccak256`` function applied to arguments encoded using ``abi.encodePacked``.
Here is a JavaScript function that creates the proper signature for the ``ReceiverPays`` example:

::

    // recipient is the address that should be paid.
    // amount, in wei, specifies how much ether should be sent.
    // nonce can be any unique number to prevent replay attacks
    // contractAddress is used to prevent cross-contract replay attacks
    function signPayment(recipient, amount, nonce, contractAddress, callback) {
        var hash = "0x" + abi.soliditySHA3(
            ["address", "uint256", "uint256", "address"],
            [recipient, amount, nonce, contractAddress]
        ).toString("hex");

        web3.eth.personal.sign(hash, web3.eth.defaultAccount, callback);
    }

Recovering the Message Signer in Solidity
-----------------------------------------

In general, ECDSA signatures consist of two parameters, ``r`` and ``s``. Signatures in Ethereum include a third parameter called ``v``, that you can use to verify which account's private key was used to sign the message, and the transaction's sender. Solidity provides a built-in function `ecrecover <mathematical-and-cryptographic-functions>`_ that accepts a message along with the ``r``, ``s`` and ``v`` parameters and returns the address that was used to sign the message.

Extracting the Signature Parameters
-----------------------------------

Signatures produced by web3.js are the concatenation of ``r``, ``s`` and ``v``, so the first step is to split these parameters apart. You can do this on the client-side, but doing it inside the smart contract means you only need to send one signature parameter rather than three. Splitting apart a byte array into component parts is a mess, so we use `inline assembly <assembly>`_ to do the job in the ``splitSignature`` function (the third function in the full contract at the end of this section).

Computing the Message Hash
--------------------------

The smart contract needs to know exactly what parameters were signed, and so it
must recreate the message from the parameters and use that for signature verification.
The functions ``prefixed`` and ``recoverSigner`` do this in the ``claimPayment`` function.

The full contract
-----------------

::

    pragma solidity >=0.4.24 <0.7.0;

    contract ReceiverPays {
        address owner = msg.sender;

        mapping(uint256 => bool) usedNonces;

        constructor() public payable {}

        function claimPayment(uint256 amount, uint256 nonce, bytes memory signature) public {
            require(!usedNonces[nonce]);
            usedNonces[nonce] = true;

            // this recreates the message that was signed on the client
            bytes32 message = prefixed(keccak256(abi.encodePacked(msg.sender, amount, nonce, this)));

            require(recoverSigner(message, signature) == owner);

            msg.sender.transfer(amount);
        }

        /// destroy the contract and reclaim the leftover funds.
        function kill() public {
            require(msg.sender == owner);
            selfdestruct(msg.sender);
        }

        /// signature methods.
        function splitSignature(bytes memory sig)
            internal
            pure
            returns (uint8 v, bytes32 r, bytes32 s)
        {
            require(sig.length == 65);

            assembly {
                // first 32 bytes, after the length prefix.
                r := mload(add(sig, 32))
                // second 32 bytes.
                s := mload(add(sig, 64))
                // final byte (first byte of the next 32 bytes).
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

        /// builds a prefixed hash to mimic the behavior of eth_sign.
        function prefixed(bytes32 hash) internal pure returns (bytes32) {
            return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", hash));
        }
    }


Writing a Simple Payment Channel
================================

Alice now builds a simple but complete implementation of a payment channel. Payment channels use cryptographic signatures to make repeated transfers of Ether securely, instantaneously, and without transaction fees.

What is a Payment Channel?
--------------------------

Payment channels allow participants to make repeated transfers of Ether without using transactions. This means that you can avoid the delays and fees associated with transactions. We are going to explore a simple unidirectional payment channel between two parties (Alice and Bob). It involves three steps:

    1. Alice funds a smart contract with Ether. This "opens" the payment channel.
    2. Alice signs messages that specify how much of that Ether is owed to the recipient. This step is repeated for each payment.
    3. Bob "closes" the payment channel, withdrawing their portion of the Ether and sending the remainder back to the sender.

.. note::
  Only steps 1 and 3 require Ethereum transactions, step 2 means that the sender transmits a cryptographically signed message to the recipient via off chain methods (e.g. email). This means only two transactions are required to support any number of transfers.

Bob is guaranteed to receive their funds because the smart contract escrows the Ether and honours a valid signed message. The smart contract also enforces a timeout, so Alice is guaranteed to eventually recover their funds even if the recipient refuses to close the channel. It is up to the participants in a payment channel to decide how long to keep it open. For a short-lived transaction, such as paying an internet café for each minute of network access, or for a longer relationship, such as paying an employee an hourly wage, a payment could last for months or years.

Opening the Payment Channel
---------------------------

To open the payment channel, Alice deploys the smart contract, attaching the Ether to be escrowed and specifying the intended recipient and a maximum duration for the channel to exist. This is the function ``SimplePaymentChannel`` in the contract, at the end of this section.

Making Payments
---------------

Alice makes payments by sending signed messages to Bob.
This step is performed entirely outside of the Ethereum network.
Messages are cryptographically signed by the sender and then transmitted directly to the recipient.

Each message includes the following information:

    * The smart contract's address, used to prevent cross-contract replay attacks.
    * The total amount of Ether that is owed the recipient so far.

A payment channel is closed just once, at the end of a series of transfers.
Because of this, only one of the messages sent is redeemed. This is why
each message specifies a cumulative total amount of Ether owed, rather than the
amount of the individual micropayment. The recipient will naturally choose to
redeem the most recent message because that is the one with the highest total.
The nonce per-message is not needed anymore, because the smart contract only honors a single message. The address of the smart contract is still used
to prevent a message intended for one payment channel from being used for a different channel.

Here is the modified JavaScript code to cryptographically sign a message from the previous section:

::

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

    // contractAddress is used to prevent cross-contract replay attacks.
    // amount, in wei, specifies how much Ether should be sent.

    function signPayment(contractAddress, amount, callback) {
        var message = constructPaymentMessage(contractAddress, amount);
        signMessage(message, callback);
    }


Closing the Payment Channel
---------------------------

When Bob is ready to receive their funds, it is time to
close the payment channel by calling a ``close`` function on the smart contract.
Closing the channel pays the recipient the Ether they are owed and destroys the contract, sending any remaining Ether back to Alice. To close the channel, Bob needs to provide a message signed by Alice.

The smart contract must verify that the message contains a valid signature from the sender.
The process for doing this verification is the same as the process the recipient uses.
The Solidity functions ``isValidSignature`` and ``recoverSigner`` work just like their
JavaScript counterparts in the previous section, with the latter function borrowed from the ``ReceiverPays`` contract.

Only the payment channel recipient can call the ``close`` function,
who naturally passes the most recent payment message because that message
carries the highest total owed. If the sender were allowed to call this function,
they could provide a message with a lower amount and cheat the recipient out of what they are owed.

The function verifies the signed message matches the given parameters.
If everything checks out, the recipient is sent their portion of the Ether,
and the sender is sent the rest via a ``selfdestruct``.
You can see the ``close`` function in the full contract.

Channel Expiration
-------------------

Bob can close the payment channel at any time, but if they fail to do so,
Alice needs a way to recover their escrowed funds. An *expiration* time was set
at the time of contract deployment. Once that time is reached, Alice can call
``claimTimeout`` to recover their funds. You can see the ``claimTimeout`` function in the full contract.

After this function is called, Bob can no longer receive any Ether,
so it is important that Bob closes the channel before the expiration is reached.

The full contract
-----------------

::

    pragma solidity >=0.4.24 <0.7.0;

    contract SimplePaymentChannel {
        address payable public sender;      // The account sending payments.
        address payable public recipient;   // The account receiving the payments.
        uint256 public expiration;  // Timeout in case the recipient never closes.

        constructor (address payable _recipient, uint256 duration)
            public
            payable
        {
            sender = msg.sender;
            recipient = _recipient;
            expiration = now + duration;
        }

        function isValidSignature(uint256 amount, bytes memory signature)
            internal
            view
            returns (bool)
        {
            bytes32 message = prefixed(keccak256(abi.encodePacked(this, amount)));

            // check that the signature is from the payment sender
            return recoverSigner(message, signature) == sender;
        }

        /// the recipient can close the channel at any time by presenting a
        /// signed amount from the sender. the recipient will be sent that amount,
        /// and the remainder will go back to the sender
        function close(uint256 amount, bytes memory signature) public {
            require(msg.sender == recipient);
            require(isValidSignature(amount, signature));

            recipient.transfer(amount);
            selfdestruct(sender);
        }

        /// the sender can extend the expiration at any time
        function extend(uint256 newExpiration) public {
            require(msg.sender == sender);
            require(newExpiration > expiration);

            expiration = newExpiration;
        }

        /// if the timeout is reached without the recipient closing the channel,
        /// then the Ether is released back to the sender.
        function claimTimeout() public {
            require(now >= expiration);
            selfdestruct(sender);
        }

        /// All functions below this are just taken from the chapter
        /// 'creating and verifying signatures' chapter.

        function splitSignature(bytes memory sig)
            internal
            pure
            returns (uint8 v, bytes32 r, bytes32 s)
        {
            require(sig.length == 65);

            assembly {
                // first 32 bytes, after the length prefix
                r := mload(add(sig, 32))
                // second 32 bytes
                s := mload(add(sig, 64))
                // final byte (first byte of the next 32 bytes)
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

        /// builds a prefixed hash to mimic the behavior of eth_sign.
        function prefixed(bytes32 hash) internal pure returns (bytes32) {
            return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", hash));
        }
    }


.. note::
  The function ``splitSignature`` does not use all security
  checks. A real implementation should use a more rigorously tested library,
  such as openzepplin's `version  <https://github.com/OpenZeppelin/openzeppelin-solidity/blob/master/contracts/ECRecovery.sol>`_ of this code.

Verifying Payments
------------------

Unlike in the previous section, messages in a payment channel aren't
redeemed right away. The recipient keeps track of the latest message and
redeems it when it's time to close the payment channel. This means it's
critical that the recipient perform their own verification of each message.
Otherwise there is no guarantee that the recipient will be able to get paid
in the end.

The recipient should verify each message using the following process:

    1. Verify that the contact address in the message matches the payment channel.
    2. Verify that the new total is the expected amount.
    3. Verify that the new total does not exceed the amount of Ether escrowed.
    4. Verify that the signature is valid and comes from the payment channel sender.

We'll use the `ethereumjs-util <https://github.com/ethereumjs/ethereumjs-util>`_
library to write this verification. The final step can be done a number of ways,
and we use JavaScript. The following code borrows the `constructMessage` function from the signing **JavaScript code** above:

::

    // this mimics the prefixing behavior of the eth_sign JSON-RPC method.
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
