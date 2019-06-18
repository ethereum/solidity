#############
Miscellaneous
#############

.. index:: storage, state variable, mapping

************************************
Layout of State Variables in Storage
************************************

Statically-sized variables (everything except mapping and dynamically-sized array types) are laid out contiguously in storage starting from position ``0``. Multiple, contiguous items that need less than 32 bytes are packed into a single storage slot if possible, according to the following rules:

- The first item in a storage slot is stored lower-order aligned.
- Elementary types use only as many bytes as are necessary to store them.
- If an elementary type does not fit the remaining part of a storage slot, it is moved to the next storage slot.
- Structs and array data always start a new slot and occupy whole slots (but items inside a struct or array are packed tightly according to these rules).

For contracts that use inheritance, the ordering of state variables is determined by the
C3-linearized order of contracts starting with the most base-ward contract. If allowed
by the above rules, state variables from different contracts do share the same storage slot.

The elements of structs and arrays are stored after each other, just as if they were given explicitly.

.. warning::
    When using elements that are smaller than 32 bytes, your contract's gas usage may be higher.
    This is because the EVM operates on 32 bytes at a time. Therefore, if the element is smaller
    than that, the EVM must use more operations in order to reduce the size of the element from 32
    bytes to the desired size.

    It is only beneficial to use reduced-size arguments if you are dealing with storage values
    because the compiler will pack multiple elements into one storage slot, and thus, combine
    multiple reads or writes into a single operation. When dealing with function arguments or memory
    values, there is no inherent benefit because the compiler does not pack these values.

    Finally, in order to allow the EVM to optimize for this, ensure that you try to order your
    storage variables and ``struct`` members such that they can be packed tightly. For example,
    declaring your storage variables in the order of ``uint128, uint128, uint256`` instead of
    ``uint128, uint256, uint128``, as the former will only take up two slots of storage whereas the
    latter will take up three.

.. note::
     The layout of state variables in storage is considered to be part of the external interface
     of Solidity due to the fact that storage pointers can be passed to libraries. This means that
     any change to the rules outlined in this section is considered a breaking change
     of the language and due to its critical nature should be considered very carefully before
     being executed.


Mappings and Dynamic Arrays
===========================

Due to their unpredictable size, mapping and dynamically-sized array types use a Keccak-256 hash
computation to find the starting position of the value or the array data. These starting positions are always full stack slots.

The mapping or the dynamic array itself occupies a slot in storage at some position ``p``
according to the above rule (or by recursively applying this rule for mappings of mappings or arrays of arrays). For dynamic arrays,
this slot stores the number of elements in the array (byte arrays and strings are an exception, see :ref:`below <bytes-and-string>`).
For mappings, the slot is unused (but it is needed so that two equal mappings after each other will use a different
hash distribution). Array data is located at ``keccak256(p)`` and the value corresponding to a mapping key
``k`` is located at ``keccak256(k . p)`` where ``.`` is concatenation. If the value is again a
non-elementary type, the positions are found by adding an offset of ``keccak256(k . p)``.

So for the following contract snippet::

    pragma solidity >=0.4.0 <0.7.0;


    contract C {
        struct S { uint a; uint b; }
        uint x;
        mapping(uint => mapping(uint => S)) data;
    }

The position of ``data[4][9].b`` is at ``keccak256(uint256(9) . keccak256(uint256(4) . uint256(1))) + 1``.

.. _bytes-and-string:

``bytes`` and ``string``
------------------------

``bytes`` and ``string`` are encoded identically. For short byte arrays, they store their data in the same
slot where the length is also stored. In particular: if the data is at most ``31`` bytes long, it is stored
in the higher-order bytes (left aligned) and the lowest-order byte stores ``length * 2``.
For byte arrays that store data which is ``32`` or more bytes long, the main slot stores ``length * 2 + 1`` and the data is
stored as usual in ``keccak256(slot)``. This means that you can distinguish a short array from a long array
by checking if the lowest bit is set: short (not set) and long (set).

.. note::
  Handling invalidly encoded slots is currently not supported but may be added in the future.

.. index: memory layout

****************
Layout in Memory
****************

Solidity reserves four 32-byte slots, with specific byte ranges (inclusive of endpoints) being used as follows:

- ``0x00`` - ``0x3f`` (64 bytes): scratch space for hashing methods
- ``0x40`` - ``0x5f`` (32 bytes): currently allocated memory size (aka. free memory pointer)
- ``0x60`` - ``0x7f`` (32 bytes): zero slot

Scratch space can be used between statements (i.e. within inline assembly). The zero slot
is used as initial value for dynamic memory arrays and should never be written to
(the free memory pointer points to ``0x80`` initially).

Solidity always places new objects at the free memory pointer and memory is never freed (this might change in the future).

.. warning::
  There are some operations in Solidity that need a temporary memory area larger than 64 bytes and therefore will not fit into the scratch space. They will be placed where the free memory points to, but given their short lifetime, the pointer is not updated. The memory may or may not be zeroed out. Because of this, one shouldn't expect the free memory to point to zeroed out memory.

  While it may seem like a good idea to use ``msize`` to arrive at a definitely zeroed out memory area, using such a pointer non-temporarily without updating the free memory pointer can have adverse results.

.. index: calldata layout

*******************
Layout of Call Data
*******************

The input data for a function call is assumed to be in the format defined by the :ref:`ABI
specification <ABI>`. Among others, the ABI specification requires arguments to be padded to multiples of 32
bytes. The internal function calls use a different convention.

Arguments for the constructor of a contract are directly appended at the end of the
contract's code, also in ABI encoding. The constructor will access them through a hard-coded offset, and
not by using the ``codesize`` opcode, since this of course changes when appending
data to the code.


.. index: variable cleanup

*********************************
Internals - Cleaning Up Variables
*********************************

When a value is shorter than 256-bit, in some cases the remaining bits
must be cleaned.
The Solidity compiler is designed to clean such remaining bits before any operations
that might be adversely affected by the potential garbage in the remaining bits.
For example, before writing a value to the memory, the remaining bits need
to be cleared because the memory contents can be used for computing
hashes or sent as the data of a message call.  Similarly, before
storing a value in the storage, the remaining bits need to be cleaned
because otherwise the garbled value can be observed.

On the other hand, we do not clean the bits if the immediately
following operation is not affected.  For instance, since any non-zero
value is considered ``true`` by ``JUMPI`` instruction, we do not clean
the boolean values before they are used as the condition for
``JUMPI``.

In addition to the design principle above, the Solidity compiler
cleans input data when it is loaded onto the stack.

Different types have different rules for cleaning up invalid values:

+---------------+---------------+-------------------+
|Type           |Valid Values   |Invalid Values Mean|
+===============+===============+===================+
|enum of n      |0 until n - 1  |exception          |
|members        |               |                   |
+---------------+---------------+-------------------+
|bool           |0 or 1         |1                  |
+---------------+---------------+-------------------+
|signed integers|sign-extended  |currently silently |
|               |word           |wraps; in the      |
|               |               |future exceptions  |
|               |               |will be thrown     |
|               |               |                   |
|               |               |                   |
+---------------+---------------+-------------------+
|unsigned       |higher bits    |currently silently |
|integers       |zeroed         |wraps; in the      |
|               |               |future exceptions  |
|               |               |will be thrown     |
+---------------+---------------+-------------------+

.. index:: optimizer, common subexpression elimination, constant propagation

*************************
Internals - The Optimiser
*************************

The Solidity optimiser operates on assembly so that other languages can use it. It splits the sequence of instructions into basic blocks at ``JUMPs`` and ``JUMPDESTs``. Inside these blocks, the optimiser analyses the instructions and records every modification to the stack, memory, or storage as an expression which consists of an instruction and a list of arguments which are pointers to other expressions. The optimiser uses a component called "CommonSubexpressionEliminator" that amongst other tasks, finds expressions that are always equal (on every input) and combines them into an expression class. The optimiser first tries to find each new expression in a list of already known expressions. If this does not work, it simplifies the expression according to rules like ``constant + constant = sum_of_constants`` or ``X * 1 = X``. Since this is a recursive process, we can also apply the latter rule if the second factor is a more complex expression where we know that it always evaluates to one. Modifications to storage and memory locations have to erase knowledge about storage and memory locations which are not known to be different. If we first write to location x and then to location y and both are input variables, the second could overwrite the first, so we do not know what is stored at x after we wrote to y. If simplification of the expression x - y evaluates to a non-zero constant, we know that we can keep our knowledge about what is stored at x.

After this process, we know which expressions have to be on the stack at the end, and have a list of modifications to memory and storage. This information is stored together with the basic blocks and is used to link them. Furthermore, knowledge about the stack, storage and memory configuration is forwarded to the next block(s). If we know the targets of all ``JUMP`` and ``JUMPI`` instructions, we can build a complete control flow graph of the program. If there is only one target we do not know (this can happen as in principle, jump targets can be computed from inputs), we have to erase all knowledge about the input state of a block as it can be the target of the unknown ``JUMP``. If the optimiser finds a ``JUMPI`` whose condition evaluates to a constant, it transforms it to an unconditional jump.

As the last step, the code in each block is re-generated. The optimiser creates a dependency graph from the expressions on the stack at the end of the block, and it drops every operation that is not part of this graph. It generates code that applies the modifications to memory and storage in the order they were made in the original code (dropping modifications which were found not to be needed). Finally, it generates all values that are required to be on the stack in the correct place.

These steps are applied to each basic block and the newly generated code is used as replacement if it is smaller. If a basic block is split at a ``JUMPI`` and during the analysis, the condition evaluates to a constant, the ``JUMPI`` is replaced depending on the value of the constant. Thus code like

::

    uint x = 7;
    data[7] = 9;
    if (data[x] != x + 2)
      return 2;
    else
      return 1;

still simplifies to code which you can compile even though the instructions contained
a jump in the beginning of the process:

::

    data[7] = 9;
    return 1;

.. index:: source mappings

***************
Source Mappings
***************

As part of the AST output, the compiler provides the range of the source
code that is represented by the respective node in the AST. This can be
used for various purposes ranging from static analysis tools that report
errors based on the AST and debugging tools that highlight local variables
and their uses.

Furthermore, the compiler can also generate a mapping from the bytecode
to the range in the source code that generated the instruction. This is again
important for static analysis tools that operate on bytecode level and
for displaying the current position in the source code inside a debugger
or for breakpoint handling.

Both kinds of source mappings use integer identifiers to refer to source files.
The identifier of a source file is stored in
``output['sources'][sourceName]['id']`` where ``output`` is the output of the
standard-json compiler interface parsed as JSON.

.. note ::
    In the case of instructions that are not associated with any particular source file,
    the source mapping assigns an integer identifier of ``-1``. This may happen for
    bytecode sections stemming from compiler-generated inline assembly statements.

The source mappings inside the AST use the following
notation:

``s:l:f``

Where ``s`` is the byte-offset to the start of the range in the source file,
``l`` is the length of the source range in bytes and ``f`` is the source
index mentioned above.

The encoding in the source mapping for the bytecode is more complicated:
It is a list of ``s:l:f:j`` separated by ``;``. Each of these
elements corresponds to an instruction, i.e. you cannot use the byte offset
but have to use the instruction offset (push instructions are longer than a single byte).
The fields ``s``, ``l`` and ``f`` are as above and ``j`` can be either
``i``, ``o`` or ``-`` signifying whether a jump instruction goes into a
function, returns from a function or is a regular jump as part of e.g. a loop.

In order to compress these source mappings especially for bytecode, the
following rules are used:

 - If a field is empty, the value of the preceding element is used.
 - If a ``:`` is missing, all following fields are considered empty.

This means the following source mappings represent the same information:

``1:2:1;1:9:1;2:1:2;2:1:2;2:1:2``

``1:2:1;:9;2:1:2;;``

***************
Tips and Tricks
***************

* Use ``delete`` on arrays to delete all its elements.
* Use shorter types for struct elements and sort them such that short types are grouped together. This can lower the gas costs as multiple ``SSTORE`` operations might be combined into a single (``SSTORE`` costs 5000 or 20000 gas, so this is what you want to optimise). Use the gas price estimator (with optimiser enabled) to check!
* Make your state variables public - the compiler creates :ref:`getters <visibility-and-getters>` for you automatically.
* If you end up checking conditions on input or state a lot at the beginning of your functions, try using :ref:`modifiers`.
* Initialize storage structs with a single assignment: ``x = MyStruct({a: 1, b: 2});``

.. note::
    If the storage struct has tightly packed properties, initialize it with separate assignments: ``x.a = 1; x.b = 2;``. In this way it will be easier for the optimizer to update storage in one go, thus making assignment cheaper.

**********
Cheatsheet
**********

.. index:: precedence

.. _order:

Order of Precedence of Operators
================================

The following is the order of precedence for operators, listed in order of evaluation.

+------------+-------------------------------------+--------------------------------------------+
| Precedence | Description                         | Operator                                   |
+============+=====================================+============================================+
| *1*        | Postfix increment and decrement     | ``++``, ``--``                             |
+            +-------------------------------------+--------------------------------------------+
|            | New expression                      | ``new <typename>``                         |
+            +-------------------------------------+--------------------------------------------+
|            | Array subscripting                  | ``<array>[<index>]``                       |
+            +-------------------------------------+--------------------------------------------+
|            | Member access                       | ``<object>.<member>``                      |
+            +-------------------------------------+--------------------------------------------+
|            | Function-like call                  | ``<func>(<args...>)``                      |
+            +-------------------------------------+--------------------------------------------+
|            | Parentheses                         | ``(<statement>)``                          |
+------------+-------------------------------------+--------------------------------------------+
| *2*        | Prefix increment and decrement      | ``++``, ``--``                             |
+            +-------------------------------------+--------------------------------------------+
|            | Unary minus                         | ``-``                                      |
+            +-------------------------------------+--------------------------------------------+
|            | Unary operations                    | ``delete``                                 |
+            +-------------------------------------+--------------------------------------------+
|            | Logical NOT                         | ``!``                                      |
+            +-------------------------------------+--------------------------------------------+
|            | Bitwise NOT                         | ``~``                                      |
+------------+-------------------------------------+--------------------------------------------+
| *3*        | Exponentiation                      | ``**``                                     |
+------------+-------------------------------------+--------------------------------------------+
| *4*        | Multiplication, division and modulo | ``*``, ``/``, ``%``                        |
+------------+-------------------------------------+--------------------------------------------+
| *5*        | Addition and subtraction            | ``+``, ``-``                               |
+------------+-------------------------------------+--------------------------------------------+
| *6*        | Bitwise shift operators             | ``<<``, ``>>``                             |
+------------+-------------------------------------+--------------------------------------------+
| *7*        | Bitwise AND                         | ``&``                                      |
+------------+-------------------------------------+--------------------------------------------+
| *8*        | Bitwise XOR                         | ``^``                                      |
+------------+-------------------------------------+--------------------------------------------+
| *9*        | Bitwise OR                          | ``|``                                      |
+------------+-------------------------------------+--------------------------------------------+
| *10*       | Inequality operators                | ``<``, ``>``, ``<=``, ``>=``               |
+------------+-------------------------------------+--------------------------------------------+
| *11*       | Equality operators                  | ``==``, ``!=``                             |
+------------+-------------------------------------+--------------------------------------------+
| *12*       | Logical AND                         | ``&&``                                     |
+------------+-------------------------------------+--------------------------------------------+
| *13*       | Logical OR                          | ``||``                                     |
+------------+-------------------------------------+--------------------------------------------+
| *14*       | Ternary operator                    | ``<conditional> ? <if-true> : <if-false>`` |
+            +-------------------------------------+--------------------------------------------+
|            | Assignment operators                | ``=``, ``|=``, ``^=``, ``&=``, ``<<=``,    |
|            |                                     | ``>>=``, ``+=``, ``-=``, ``*=``, ``/=``,   |
|            |                                     | ``%=``                                     |
+------------+-------------------------------------+--------------------------------------------+
| *15*       | Comma operator                      | ``,``                                      |
+------------+-------------------------------------+--------------------------------------------+

.. index:: assert, block, coinbase, difficulty, number, block;number, timestamp, block;timestamp, msg, data, gas, sender, value, now, gas price, origin, revert, require, keccak256, ripemd160, sha256, ecrecover, addmod, mulmod, cryptography, this, super, selfdestruct, balance, send

Global Variables
================

- ``abi.decode(bytes memory encodedData, (...)) returns (...)``: :ref:`ABI <ABI>`-decodes the provided data. The types are given in parentheses as second argument. Example: ``(uint a, uint[2] memory b, bytes memory c) = abi.decode(data, (uint, uint[2], bytes))``
- ``abi.encode(...) returns (bytes memory)``: :ref:`ABI <ABI>`-encodes the given arguments
- ``abi.encodePacked(...) returns (bytes memory)``: Performs :ref:`packed encoding <abi_packed_mode>` of the given arguments. Note that this encoding can be ambiguous!
- ``abi.encodeWithSelector(bytes4 selector, ...) returns (bytes memory)``: :ref:`ABI <ABI>`-encodes the given arguments
   starting from the second and prepends the given four-byte selector
- ``abi.encodeWithSignature(string memory signature, ...) returns (bytes memory)``: Equivalent to ``abi.encodeWithSelector(bytes4(keccak256(bytes(signature)), ...)```
- ``block.coinbase`` (``address payable``): current block miner's address
- ``block.difficulty`` (``uint``): current block difficulty
- ``block.gaslimit`` (``uint``): current block gaslimit
- ``block.number`` (``uint``): current block number
- ``block.timestamp`` (``uint``): current block timestamp
- ``gasleft() returns (uint256)``: remaining gas
- ``msg.data`` (``bytes``): complete calldata
- ``msg.sender`` (``address payable``): sender of the message (current call)
- ``msg.value`` (``uint``): number of wei sent with the message
- ``now`` (``uint``): current block timestamp (alias for ``block.timestamp``)
- ``tx.gasprice`` (``uint``): gas price of the transaction
- ``tx.origin`` (``address payable``): sender of the transaction (full call chain)
- ``assert(bool condition)``: abort execution and revert state changes if condition is ``false`` (use for internal error)
- ``require(bool condition)``: abort execution and revert state changes if condition is ``false`` (use for malformed input or error in external component)
- ``require(bool condition, string memory message)``: abort execution and revert state changes if condition is ``false`` (use for malformed input or error in external component). Also provide error message.
- ``revert()``: abort execution and revert state changes
- ``revert(string memory message)``: abort execution and revert state changes providing an explanatory string
- ``blockhash(uint blockNumber) returns (bytes32)``: hash of the given block - only works for 256 most recent blocks
- ``keccak256(bytes memory) returns (bytes32)``: compute the Keccak-256 hash of the input
- ``sha256(bytes memory) returns (bytes32)``: compute the SHA-256 hash of the input
- ``ripemd160(bytes memory) returns (bytes20)``: compute the RIPEMD-160 hash of the input
- ``ecrecover(bytes32 hash, uint8 v, bytes32 r, bytes32 s) returns (address)``: recover address associated with the public key from elliptic curve signature, return zero on error
- ``addmod(uint x, uint y, uint k) returns (uint)``: compute ``(x + y) % k`` where the addition is performed with arbitrary precision and does not wrap around at ``2**256``. Assert that ``k != 0`` starting from version 0.5.0.
- ``mulmod(uint x, uint y, uint k) returns (uint)``: compute ``(x * y) % k`` where the multiplication is performed with arbitrary precision and does not wrap around at ``2**256``. Assert that ``k != 0`` starting from version 0.5.0.
- ``this`` (current contract's type): the current contract, explicitly convertible to ``address`` or ``address payable``
- ``super``: the contract one level higher in the inheritance hierarchy
- ``selfdestruct(address payable recipient)``: destroy the current contract, sending its funds to the given address
- ``<address>.balance`` (``uint256``): balance of the :ref:`address` in Wei
- ``<address payable>.send(uint256 amount) returns (bool)``: send given amount of Wei to :ref:`address`, returns ``false`` on failure
- ``<address payable>.transfer(uint256 amount)``: send given amount of Wei to :ref:`address`, throws on failure
- ``type(C).name`` (``string``): the name of the contract
- ``type(C).creationCode`` (``bytes memory``): creation bytecode of the given contract, see :ref:`Type Information<meta-type>`.
- ``type(C).runtimeCode`` (``bytes memory``): runtime bytecode of the given contract, see :ref:`Type Information<meta-type>`.

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
    In version 0.5.0, the following aliases were removed: ``suicide`` as alias for ``selfdestruct``,
    ``msg.gas`` as alias for ``gasleft``, ``block.blockhash`` as alias for ``blockhash`` and
    ``sha3`` as alias for ``keccak256``.

.. index:: visibility, public, private, external, internal

Function Visibility Specifiers
==============================

::

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
- ``anonymous`` for events: Does not store event signature as topic.
- ``indexed`` for event parameters: Stores the parameter as topic.

Reserved Keywords
=================

These keywords are reserved in Solidity. They might become part of the syntax in the future:

``abstract``, ``after``, ``alias``, ``apply``, ``auto``, ``case``, ``catch``, ``copyof``, ``default``,
``define``, ``final``, ``immutable``, ``implements``, ``in``, ``inline``, ``let``, ``macro``, ``match``,
``mutable``, ``null``, ``of``, ``override``, ``partial``, ``promise``, ``reference``, ``relocatable``,
``sealed``, ``sizeof``, ``static``, ``supports``, ``switch``, ``try``, ``typedef``, ``typeof``,
``unchecked``.

Language Grammar
================

.. literalinclude:: grammar.txt
   :language: none
