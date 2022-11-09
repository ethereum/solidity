**********
Cheatsheet
**********

.. index:: operator; precedence

Order of Precedence of Operators
================================
.. include:: types/operator-precedence-table.rst

.. index:: assert, block, coinbase, difficulty, number, block;number, timestamp, block;timestamp, msg, data, gas, sender, value, gas price, origin, revert, require, keccak256, ripemd160, sha256, ecrecover, addmod, mulmod, cryptography, this, super, selfdestruct, balance, codehash, send

Global Variables
================

.. list-table::
   :widths: 20 50 30
   :header-rows: 1

   * - Global Variable
     - Result Type
     - Description
   * - ``abi.decode(bytes memory encodedData, (...))``
     - ``...``
     - :ref:`ABI <ABI>`-Decodes the provided data. The types are given in parentheses as second argument.
   * - ``abi.encode(...)``
     - ``bytes memory``
     - :ref:`ABI <ABI>`-Encodes the given arguments
   * - ``abi.encodePacked(...)``
     - ``bytes memory``
     - Performs :ref:`packed encoding <abi_packed_mode>` of the given arguments. Note that this encoding can be ambiguous!
   * - ``abi.encodeWithSelector(bytes4 selector, ...)``
     - ``bytes memory``
     - :ref:`ABI <ABI>`-Encodes the given arguments starting from the second and prepends the given four-byte selector
   * - ``abi.encodeCall(function functionPointer, (...))``
     - ``bytes memory``
     - ABI-encodes a call to ``functionPointer`` with the arguments found in the tuple. Performs a full type-check, ensuring the types match the function signature. Result equals ``abi.encodeWithSelector(functionPointer.selector, (...))``
   * - ``abi.encodeWithSignature(string memory signature, ...)``
     - ``bytes memory``
     -  Equivalent to ``abi.encodeWithSelector(bytes4(keccak256(bytes(signature)), ...)``
   * - ``bytes.concat(...)``
     - ``bytes memory``
     - :ref:`Concatenates variable number of arguments to one byte array<bytes-concat>`
   * - ``string.concat(...)``
     - ``string memory``
     - :ref:`Concatenates variable number of arguments to one string array<string-concat>`
   * - ``block.basefee``
     - ``uint``
     - Current block's base fee (`EIP-3198 <https://eips.ethereum.org/EIPS/eip-3198>`_ and `EIP-1559 <https://eips.ethereum.org/EIPS/eip-1559>`_)
   * - ``block.chainid``
     - ``uint``
     - Current chain id
   * - ``block.coinbase``
     - ``address payable``
     - Current block miner's address
   * - ``block.difficulty``
     - ``uint``
     - Current block difficulty
   * - ``block.gaslimit``
     - ``uint``
     - Current block gaslimit
   * - ``block.number``
     - ``uint``
     - Current block number
   * - ``block.timestamp``
     - ``uint``
     - Current block timestamp in seconds since Unix epoch
   * - ``gasleft()``
     - ``uint256``
     - Remaining gas
   * - ``msg.data``
     - ``bytes``
     - Complete calldata
   * - ``msg.sender``
     - ``address``
     - Sender of the message (current call)
   * - ``msg.sig``
     - ``bytes4``
     - First four bytes of the calldata (i.e. function identifier)
   * - ``msg.value``
     - ``uint``
     - Number of wei sent with the message
   * - ``tx.gasprice``
     - ``uint``
     - Gas price of the transaction
   * - ``tx.origin``
     - ``address``
     - Sender of the transaction (full call chain)
   * - ``assert(bool condition)``
     -
     - Abort execution and revert state changes if condition is ``false`` (use for internal error)
   * - ``require(bool condition)``
     -
     - Abort execution and revert state changes if condition is ``false`` (use for malformed input or error in external component)
   * - ``require(bool condition, string memory message)``
     -
     - Abort execution and revert state changes if condition is ``false`` (use for malformed input or error in external component). Also provide error message.
   * - ``revert()``
     -
     - Abort execution and revert state changes
   * - ``revert(string memory message)``
     -
     - Abort execution and revert state changes providing an explanatory string
   * - ``blockhash(uint blockNumber)``
     - ``bytes32``
     - Hash of the given block - only works for 256 most recent blocks
   * - ``keccak256(bytes memory)``
     - ``bytes32``
     - Compute the Keccak-256 hash of the input
   * - ``sha256(bytes memory)``
     - ``bytes32``
     - Compute the SHA-256 hash of the input
   * - ``ripemd160(bytes memory)``
     - ``bytes20``
     - Compute the RIPEMD-160 hash of the input
   * - ``ecrecover(bytes32 hash, uint8 v, bytes32 r, bytes32 s)``
     - ``address``
     - Recover address associated with the public key from elliptic curve signature, return zero on error
   * - ``addmod(uint x, uint y, uint k)``
     - ``uint``
     - Compute ``(x + y) % k`` where the addition is performed with arbitrary precision and does not wrap around at ``2**256``. Assert that ``k != 0`` starting from version 0.5.0.
   * - ``mulmod(uint x, uint y, uint k)``
     - ``uint``
     - Compute ``(x * y) % k`` where the multiplication is performed with arbitrary precision and does not wrap around at ``2**256``. Assert that ``k != 0`` starting from version 0.5.0.
   * - ``this``
     - current contract's type
     - The current contract, explicitly convertible to ``address`` or ``address payable``
   * - ``super``
     -
     - The contract one level higher in the inheritance hierarchy
   * - ``selfdestruct(address payable recipient)``
     -
     - Destroy the current contract, sending its funds to the given address
   * - ``<address>.balance``
     - ``uint256``
     - Balance of the :ref:`address` in Wei
   * - ``<address>.code``
     - ``bytes memory``
     - Code at the :ref:`address` (can be empty)
   * - ``<address>.codehash``
     - ``bytes32``
     - The codehash of the :ref:`address`
   * - ``<address payable>.send(uint256 amount)``
     - ``bool``
     - Send given amount of Wei to :ref:`address`, returns ``false`` on failure
   * - ``<address payable>.transfer(uint256 amount)``
     -
     - Send given amount of Wei to :ref:`address`, throws on failure
   * - ``type(C).name``
     - ``string``
     - The name of the contract
   * - ``type(C).creationCode``
     - ``bytes memory``
     - Creation bytecode of the given contract, see :ref:`Type Information<meta-type>`.
   * - ``type(C).runtimeCode``
     - ``bytes memory``
     - Runtime bytecode of the given contract, see :ref:`Type Information<meta-type>`.
   * - ``type(I).interfaceId``
     - ``bytes4``
     - Value containing the EIP-165 interface identifier of the given interface, see :ref:`Type Information<meta-type>`.
   * - ``type(T).min``
     - ``T``
     - The minimum value representable by the integer type ``T``, see :ref:`Type Information<meta-type>`.
   * - ``type(T).max``
     - ``T``
     - The maximum value representable by the integer type ``T``, see :ref:`Type Information<meta-type>`.


.. index:: visibility, public, private, external, internal

Function Visibility Specifiers
==============================

.. code-block:: solidity
    :force:

    function myFunction() <visibility specifier> returns (bool) {
        return true;
    }

- ``public``: visible externally and internally (creates a :ref:`getter function<getter-functions>` for storage/state variables)
- ``private``: only visible in the current contract
- ``external``: only visible externally (only for functions) - i.e. can only be message-called (via ``this.func``)
- ``internal``: only visible internally


.. index:: modifiers, pure, view, payable, constant, anonymous, indexed

Modifiers
=========

- ``pure`` for functions: Disallows modification or access of state.
- ``view`` for functions: Disallows modification of state.
- ``payable`` for functions: Allows them to receive Ether together with a call.
- ``constant`` for state variables: Disallows assignment (except initialisation), does not occupy storage slot.
- ``immutable`` for state variables: Allows exactly one assignment at construction time and is constant afterwards. Is stored in code.
- ``anonymous`` for events: Does not store event signature as topic.
- ``indexed`` for event parameters: Stores the parameter as topic.
- ``virtual`` for functions and modifiers: Allows the function's or modifier's
  behaviour to be changed in derived contracts.
- ``override``: States that this function, modifier or public state variable changes
  the behaviour of a function or modifier in a base contract.

