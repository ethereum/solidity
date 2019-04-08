**************************************
Units and Globally Available Variables
**************************************

.. index:: wei, finney, szabo, ether

Ether Units
===========

A literal number can take a suffix of ``wei``, ``finney``, ``szabo`` or ``ether`` to specify a subdenomination of Ether, where Ether numbers without a postfix are assumed to be Wei.

::

    assert(1 wei == 1);
    assert(1 szabo == 1e12);
    assert(1 finney == 1e15);
    assert(1 ether == 1e18);

The only effect of the subdenomination suffix is a multiplication by a power of ten.


.. index:: time, seconds, minutes, hours, days, weeks, years

Time Units
==========

Suffixes like ``seconds``, ``minutes``, ``hours``, ``days`` and ``weeks``
after literal numbers can be used to specify units of time where seconds are the base
unit and units are considered naively in the following way:

 * ``1 == 1 seconds``
 * ``1 minutes == 60 seconds``
 * ``1 hours == 60 minutes``
 * ``1 days == 24 hours``
 * ``1 weeks == 7 days``

Take care if you perform calendar calculations using these units, because
not every year equals 365 days and not even every day has 24 hours
because of `leap seconds <https://en.wikipedia.org/wiki/Leap_second>`_.
Due to the fact that leap seconds cannot be predicted, an exact calendar
library has to be updated by an external oracle.

.. note::
    The suffix ``years`` has been removed in version 0.5.0 due to the reasons above.

These suffixes cannot be applied to variables. For example, if you want to
interpret a function parameter in days, you can in the following way::

    function f(uint start, uint daysAfter) public {
        if (now >= start + daysAfter * 1 days) {
          // ...
        }
    }

Special Variables and Functions
===============================

There are special variables and functions which always exist in the global
namespace and are mainly used to provide information about the blockchain
or are general-use utility functions.

.. index:: abi, block, coinbase, difficulty, encode, number, block;number, timestamp, block;timestamp, msg, data, gas, sender, value, now, gas price, origin


Block and Transaction Properties
--------------------------------

- ``blockhash(uint blockNumber) returns (bytes32)``: hash of the given block - only works for 256 most recent, excluding current, blocks
- ``block.coinbase`` (``address payable``): current block miner's address
- ``block.difficulty`` (``uint``): current block difficulty
- ``block.gaslimit`` (``uint``): current block gaslimit
- ``block.number`` (``uint``): current block number
- ``block.timestamp`` (``uint``): current block timestamp as seconds since unix epoch
- ``gasleft() returns (uint256)``: remaining gas
- ``msg.data`` (``bytes calldata``): complete calldata
- ``msg.sender`` (``address payable``): sender of the message (current call)
- ``msg.sig`` (``bytes4``): first four bytes of the calldata (i.e. function identifier)
- ``msg.value`` (``uint``): number of wei sent with the message
- ``now`` (``uint``): current block timestamp (alias for ``block.timestamp``)
- ``tx.gasprice`` (``uint``): gas price of the transaction
- ``tx.origin`` (``address payable``): sender of the transaction (full call chain)

.. note::
    The values of all members of ``msg``, including ``msg.sender`` and
    ``msg.value`` can change for every **external** function call.
    This includes calls to library functions.

.. note::
    Do not rely on ``block.timestamp``, ``now`` and ``blockhash`` as a source of randomness,
    unless you know what you are doing.

    Both the timestamp and the block hash can be influenced by miners to some degree.
    Bad actors in the mining community can for example run a casino payout function on a chosen hash
    and just retry a different hash if they did not receive any money.

    The current block timestamp must be strictly larger than the timestamp of the last block,
    but the only guarantee is that it will be somewhere between the timestamps of two
    consecutive blocks in the canonical chain.

.. note::
    The block hashes are not available for all blocks for scalability reasons.
    You can only access the hashes of the most recent 256 blocks, all other
    values will be zero.

.. note::
    The function ``blockhash`` was previously known as ``block.blockhash``, which was deprecated in
    version 0.4.22 and removed in version 0.5.0.

.. note::
    The function ``gasleft`` was previously known as ``msg.gas``, which was deprecated in
    version 0.4.21 and removed in version 0.5.0.

.. index:: abi, encoding, packed

ABI Encoding and Decoding Functions
-----------------------------------

- ``abi.decode(bytes memory encodedData, (...)) returns (...)``: ABI-decodes the given data, while the types are given in parentheses as second argument. Example: ``(uint a, uint[2] memory b, bytes memory c) = abi.decode(data, (uint, uint[2], bytes))``
- ``abi.encode(...) returns (bytes memory)``: ABI-encodes the given arguments
- ``abi.encodePacked(...) returns (bytes memory)``: Performs :ref:`packed encoding <abi_packed_mode>` of the given arguments. Note that packed encoding can be ambiguous!
- ``abi.encodeWithSelector(bytes4 selector, ...) returns (bytes memory)``: ABI-encodes the given arguments starting from the second and prepends the given four-byte selector
- ``abi.encodeWithSignature(string memory signature, ...) returns (bytes memory)``: Equivalent to ``abi.encodeWithSelector(bytes4(keccak256(bytes(signature))), ...)```

.. note::
    These encoding functions can be used to craft data for external function calls without actually
    calling an external function. Furthermore, ``keccak256(abi.encodePacked(a, b))`` is a way
    to compute the hash of structured data (although be aware that it is possible to
    craft a "hash collision" using different function parameter types).

See the documentation about the :ref:`ABI <ABI>` and the
:ref:`tightly packed encoding <abi_packed_mode>` for details about the encoding.

.. index:: assert, revert, require

Error Handling
--------------

See the dedicated section on :ref:`assert and require<assert-and-require>` for
more details on error handling and when to use which function.

``assert(bool condition)``:
    causes an invalid opcode and thus state change reversion if the condition is not met - to be used for internal errors.
``require(bool condition)``:
    reverts if the condition is not met - to be used for errors in inputs or external components.
``require(bool condition, string memory message)``:
    reverts if the condition is not met - to be used for errors in inputs or external components. Also provides an error message.
``revert()``:
    abort execution and revert state changes
``revert(string memory reason)``:
    abort execution and revert state changes, providing an explanatory string

.. index:: keccak256, ripemd160, sha256, ecrecover, addmod, mulmod, cryptography,

Mathematical and Cryptographic Functions
----------------------------------------

``addmod(uint x, uint y, uint k) returns (uint)``:
    compute ``(x + y) % k`` where the addition is performed with arbitrary precision and does not wrap around at ``2**256``. Assert that ``k != 0`` starting from version 0.5.0.

``mulmod(uint x, uint y, uint k) returns (uint)``:
    compute ``(x * y) % k`` where the multiplication is performed with arbitrary precision and does not wrap around at ``2**256``. Assert that ``k != 0`` starting from version 0.5.0.

``keccak256(bytes memory) returns (bytes32)``:
    compute the Keccak-256 hash of the input

.. note::

    There used to be an alias for ``keccak256`` called ``sha3``, which was removed in version 0.5.0.

``sha256(bytes memory) returns (bytes32)``:
    compute the SHA-256 hash of the input

``ripemd160(bytes memory) returns (bytes20)``:
    compute RIPEMD-160 hash of the input

``ecrecover(bytes32 hash, uint8 v, bytes32 r, bytes32 s) returns (address)``:
    recover the address associated with the public key from elliptic curve signature or return zero on error.
    The function parameters correspond to ECDSA values of the signature:

    ``r`` = first 32 bytes of signature
    ``s`` = second 32 bytes of signature
    ``v`` = final 1 byte of signature

    ``ecrecover`` returns an ``address``, and not an ``address payable``. See :ref:`address payable<address>` for
    conversion, in case you need to transfer funds to the recovered address.

    For further details, read `example usage <https://ethereum.stackexchange.com/q/1777/222>`_.

.. warning::

    If you use ``ecrecover``, be aware that a valid signature can be turned into a different valid signature without
    requiring knowledge of the corresponding private key. Since `EIP-2 <http://eips.ethereum.org/EIPS/eip-2#specification>`_,
    all transaction signatures with an ``s`` value greater than ``secp256k1n/2`` are considered invalid.

    This is usually not a problem unless you require signatures to be unique or
    use them to identify items. OpenZeppelin have a `ECDSA helper library <https://docs.openzeppelin.org/docs/cryptography_ecdsa>`_ that you can use as a wrapper for ``ecrecover`` without this issue.

.. note::

    When running ``sha256``, ``ripemd160`` or ``ecrecover`` on a *private blockchain*, you might encounter Out-of-Gas. This is because these functions are implemented as "precompiled contracts" and only really exist after they receive the first message (although their contract code is hardcoded). Messages to non-existing contracts are more expensive and thus the execution might run into an Out-of-Gas error. A workaround for this problem is to first send Wei (1 for example) to each of the contracts before you use them in your actual contracts. This is not an issue on the main or test net.

.. index:: balance, send, transfer, call, callcode, delegatecall, staticcall

.. _address_related:

Members of Address Types
------------------------

``<address>.balance`` (``uint256``):
    balance of the :ref:`address` in Wei
``<address payable>.transfer(uint256 amount)``:
    send given amount of Wei to :ref:`address`, reverts on failure, forwards 2300 gas stipend, not adjustable
``<address payable>.send(uint256 amount) returns (bool)``:
    send given amount of Wei to :ref:`address`, returns ``false`` on failure, forwards 2300 gas stipend, not adjustable
``<address>.call(bytes memory) returns (bool, bytes memory)``:
    issue low-level ``CALL`` with the given payload, returns success condition and return data, forwards all available gas, adjustable
``<address>.delegatecall(bytes memory) returns (bool, bytes memory)``:
    issue low-level ``DELEGATECALL`` with the given payload, returns success condition and return data, forwards all available gas, adjustable
``<address>.staticcall(bytes memory) returns (bool, bytes memory)``:
    issue low-level ``STATICCALL`` with the given payload, returns success condition and return data, forwards all available gas, adjustable

For more information, see the section on :ref:`address`.

.. warning::
    You should avoid using ``.call()`` whenever possible when executing another contract function as it bypasses type checking,
    function existence check, and argument packing.

.. warning::
    There are some dangers in using ``send``: The transfer fails if the call stack depth is at 1024
    (this can always be forced by the caller) and it also fails if the recipient runs out of gas. So in order
    to make safe Ether transfers, always check the return value of ``send``, use ``transfer`` or even better:
    Use a pattern where the recipient withdraws the money.

.. note::
   Prior to version 0.5.0, Solidity allowed address members to be accessed by a contract instance, for example ``this.balance``.
   This is now forbidden and an explicit conversion to address must be done: ``address(this).balance``.

.. note::
   If state variables are accessed via a low-level delegatecall, the storage layout of the two contracts
   must align in order for the called contract to correctly access the storage variables of the calling contract by name.
   This is of course not the case if storage pointers are passed as function arguments as in the case for
   the high-level libraries.

.. note::
    Prior to version 0.5.0, ``.call``, ``.delegatecall`` and ``.staticcall`` only returned the
    success condition and not the return data.

.. note::
    Prior to version 0.5.0, there was a member called ``callcode`` with similar but slightly different
    semantics than ``delegatecall``.


.. index:: this, selfdestruct

Contract Related
----------------

``this`` (current contract's type):
    the current contract, explicitly convertible to :ref:`address`

``selfdestruct(address payable recipient)``:
    destroy the current contract, sending its funds to the given :ref:`address`

Furthermore, all functions of the current contract are callable directly including the current function.

.. note::
    Prior to version 0.5.0, there was a function called ``suicide`` with the same
    semantics as ``selfdestruct``.

.. index:: type, creationCode, runtimeCode

.. _meta-type:

Type Information
----------------

The expression ``type(X)`` can be used to retrieve information about the
type ``X``. Currently, there is limited support for this feature, but
it might be expanded in the future. The following properties are
available for a contract type ``C``:

``type(C).name``:
    The name of the contract.

``type(C).creationCode``:
    Memory byte array that contains the creation bytecode of the contract.
    This can be used in inline assembly to build custom creation routines,
    especially by using the ``create2`` opcode.
    This property can **not** be accessed in the contract itself or any
    derived contract. It causes the bytecode to be included in the bytecode
    of the call site and thus circular references like that are not possible.

``type(C).runtimeCode``:
    Memory byte array that contains the runtime bytecode of the contract.
    This is the code that is usually deployed by the constructor of ``C``.
    If ``C`` has a constructor that uses inline assembly, this might be
    different from the actually deployed bytecode. Also note that libraries
    modify their runtime bytecode at time of deployment to guard against
    regular calls.
    The same restrictions as with ``.creationCode`` also apply for this
    property.
