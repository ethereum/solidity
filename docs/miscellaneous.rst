#############
Miscellaneous
#############

.. index:: storage, state variable, mapping

************************************
Layout of State Variables in Storage
************************************

Statically-sized variables (everything except mapping and dynamically-sized array types) are laid out contiguously in storage starting from position ``0``. Multiple items that need less than 32 bytes are packed into a single storage slot if possible, according to the following rules:

- The first item in a storage slot is stored lower-order aligned.
- Elementary types use only that many bytes that are necessary to store them.
- If an elementary type does not fit the remaining part of a storage slot, it is moved to the next storage slot.
- Structs and array data always start a new slot and occupy whole slots (but items inside a struct or array are packed tightly according to these rules).

The elements of structs and arrays are stored after each other, just as if they were given explicitly.

Due to their unpredictable size, mapping and dynamically-sized array types use a ``sha3``
computation to find the starting position of the value or the array data. These starting positions are always full stack slots.

The mapping or the dynamic array itself
occupies an (unfilled) slot in storage at some position ``p`` according to the above rule (or by
recursively applying this rule for mappings to mappings or arrays of arrays). For a dynamic array, this slot stores the number of elements in the array (byte arrays and strings are an exception here, see below). For a mapping, the slot is unused (but it is needed so that two equal mappings after each other will use a different hash distribution).
Array data is located at ``sha3(p)`` and the value corresponding to a mapping key
``k`` is located at ``sha3(k . p)`` where ``.`` is concatenation. If the value is again a
non-elementary type, the positions are found by adding an offset of ``sha3(k . p)``.

``bytes`` and ``string`` store their data in the same slot where also the length is stored if they are short. In particular: If the data is at most ``31`` bytes long, it is stored in the higher-order bytes (left aligned) and the lowest-order byte stores ``length * 2``. If it is longer, the main slot stores ``length * 2 + 1`` and the data is stored as usual in ``sha3(slot)``.

So for the following contract snippet::

    contract C {
      struct s { uint a; uint b; }
      uint x;
      mapping(uint => mapping(uint => s)) data;
    }

The position of ``data[4][9].b`` is at ``sha3(uint256(9) . sha3(uint256(4) . uint256(1))) + 1``.

*****************
Esoteric Features
*****************

There are some types in Solidity's type system that have no counterpart in the syntax. One of these types are the types of functions. But still, using ``var`` it is possible to have local variables of these types::

    contract FunctionSelector {
      function select(bool useB, uint x) returns (uint z) {
        var f = a;
        if (useB) f = b;
        return f(x);
      }

      function a(uint x) returns (uint z) {
        return x * x;
      }

      function b(uint x) returns (uint z) {
        return 2 * x;
      }
    }

Calling ``select(false, x)`` will compute ``x * x`` and ``select(true, x)`` will compute ``2 * x``.

.. index:: optimizer, common subexpression elimination, constant propagation

*************************
Internals - the Optimizer
*************************

The Solidity optimizer operates on assembly, so it can be and also is used by other languages. It splits the sequence of instructions into basic blocks at JUMPs and JUMPDESTs. Inside these blocks, the instructions are analysed and every modification to the stack, to memory or storage is recorded as an expression which consists of an instruction and a list of arguments which are essentially pointers to other expressions. The main idea is now to find expressions that are always equal (on every input) and combine them into an expression class. The optimizer first tries to find each new expression in a list of already known expressions. If this does not work, the expression is simplified according to rules like ``constant + constant = sum_of_constants`` or ``X * 1 = X``. Since this is done recursively, we can also apply the latter rule if the second factor is a more complex expression where we know that it will always evaluate to one. Modifications to storage and memory locations have to erase knowledge about storage and memory locations which are not known to be different: If we first write to location x and then to location y and both are input variables, the second could overwrite the first, so we actually do not know what is stored at x after we wrote to y. On the other hand, if a simplification of the expression x - y evaluates to a non-zero constant, we know that we can keep our knowledge about what is stored at x.

At the end of this process, we know which expressions have to be on the stack in the end and have a list of modifications to memory and storage. This information is stored together with the basic blocks and is used to link them. Furthermore, knowledge about the stack, storage and memory configuration is forwarded to the next block(s). If we know the targets of all JUMP and JUMPI instructions, we can build a complete control flow graph of the program. If there is only one target we do not know (this can happen as in principle, jump targets can be computed from inputs), we have to erase all knowledge about the input state of a block as it can be the target of the unknown JUMP. If a JUMPI is found whose condition evaluates to a constant, it is transformed to an unconditional jump.

As the last step, the code in each block is completely re-generated. A dependency graph is created from the expressions on the stack at the end of the block and every operation that is not part of this graph is essentially dropped. Now code is generated that applies the modifications to memory and storage in the order they were made in the original code (dropping modifications which were found not to be needed) and finally, generates all values that are required to be on the stack in the correct place.

These steps are applied to each basic block and the newly generated code is used as replacement if it is smaller. If a basic block is split at a JUMPI and during the analysis, the condition evaluates to a constant, the JUMPI is replaced depending on the value of the constant, and thus code like

::

    var x = 7;
    data[7] = 9;
    if (data[x] != x + 2)
      return 2;
    else
      return 1;

is simplified to code which can also be compiled from

::

    data[7] = 9;
    return 1;

even though the instructions contained a jump in the beginning.

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

Both kinds of source mappings use integer indentifiers to refer to source files.
These are regular array indices into a list of source files usually called
``"sourceList"``, which is part of the combined-json and the output of
the json / npm compiler.

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

``1:2:1;:9;2::2;;``


.. index:: ! commandline compiler, compiler;commandline, ! solc, ! linker

.. _commandline-compiler:

******************************
Using the Commandline Compiler
******************************

One of the build targets of the Solidity repository is ``solc``, the solidity commandline compiler.
Using ``solc --help`` provides you with an explanation of all options. The compiler can produce various outputs, ranging from simple binaries and assembly over an abstract syntax tree (parse tree) to estimations of gas usage.
If you only want to compile a single file, you run it as ``solc --bin sourceFile.sol`` and it will print the binary. Before you deploy your contract, activate the optimizer while compiling using ``solc --optimize --bin sourceFile.sol``. If you want to get some of the more advanced output variants of ``solc``, it is probably better to tell it to output everything to separate files using ``solc -o outputDirectory --bin --ast --asm sourceFile.sol``.

The commandline compiler will automatically read imported files from the filesystem, but
it is also possible to provide path redirects using ``context:prefix=path`` in the following way:

::

    solc github.com/ethereum/dapp-bin/=/usr/local/lib/dapp-bin/ =/usr/local/lib/fallback file.sol

This essentially instructs the compiler to search for anything starting with
``github.com/ethereum/dapp-bin/`` under ``/usr/local/lib/dapp-bin`` and if it does not
find the file there, it will look at ``/usr/local/lib/fallback`` (the empty prefix
always matches). ``solc`` will not read files from the filesystem that lie outside of
the remapping targets and outside of the directories where explicitly specified source
files reside, so things like ``import "/etc/passwd";`` only work if you add ``=/`` as a remapping.

You can restrict remappings to only certain source files by prefixing a context.

The section on :ref:`import` provides more details on remappings.

If there are multiple matches due to remappings, the one with the longest common prefix is selected.

If your contracts use :ref:`libraries <libraries>`, you will notice that the bytecode contains substrings of the form ``__LibraryName______``. You can use ``solc`` as a linker meaning that it will insert the library addresses for you at those points:

Either add ``--libraries "Math:0x12345678901234567890 Heap:0xabcdef0123456"`` to your command to provide an address for each library or store the string in a file (one library per line) and run ``solc`` using ``--libraries fileName``.

If ``solc`` is called with the option ``--link``, all input files are interpreted to be unlinked binaries (hex-encoded) in the ``__LibraryName____``-format given above and are linked in-place (if the input is read from stdin, it is written to stdout). All options except ``--libraries`` are ignored (including ``-o``) in this case.

***************
Tips and Tricks
***************

* Use ``delete`` on arrays to delete all its elements.
* Use shorter types for struct elements and sort them such that short types are grouped together. This can lower the gas costs as multiple SSTORE operations might be combined into a single (SSTORE costs 5000 or 20000 gas, so this is what you want to optimise). Use the gas price estimator (with optimiser enabled) to check!
* Make your state variables public - the compiler will create :ref:`getters <visibility-and-accessors>` for you for free.
* If you end up checking conditions on input or state a lot at the beginning of your functions, try using :ref:`modifiers`.
* If your contract has a function called ``send`` but you want to use the built-in send-function, use ``address(contractVariable).send(amount)``.
* If you do **not** want your contracts to receive ether when called via ``send``, you can add a throwing fallback function ``function() { throw; }``.
* Initialise storage structs with a single assignment: ``x = MyStruct({a: 1, b: 2});``

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
|            | Function-like call                  | ``<func>(<args...>)``                      |
+            +-------------------------------------+--------------------------------------------+
|            | Array subscripting                  | ``<array>[<index>]``                       |
+            +-------------------------------------+--------------------------------------------+
|            | Member access                       | ``<object>.<member>``                      |
+            +-------------------------------------+--------------------------------------------+
|            | Parentheses                         | ``(<statement>)``                          |
+------------+-------------------------------------+--------------------------------------------+
| *2*        | Prefix increment and decrement      | ``++``, ``--``                             |
+            +-------------------------------------+--------------------------------------------+
|            | Unary plus and minus                | ``+``, ``-``                               |
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
+------------+-------------------------------------+--------------------------------------------+
| *15*       | Assignment operators                | ``=``, ``|=``, ``^=``, ``&=``, ``<<=``,    |
|            |                                     | ``>>=``, ``+=``, ``-=``, ``*=``, ``/=``,   |
|            |                                     | ``%=``                                     |
+------------+-------------------------------------+--------------------------------------------+
| *16*       | Comma operator                      | ``,``                                      |
+------------+-------------------------------------+--------------------------------------------+

.. index:: block, coinbase, difficulty, number, block;number, timestamp, block;timestamp, msg, data, gas, sender, value, now, gas price, origin, sha3, ripemd160, sha256, ecrecover, addmod, mulmod, cryptography, this, super, selfdestruct, balance, send

Global Variables
================

- ``block.blockhash(uint blockNumber) returns (bytes32)``: hash of the given block - only works for 256 most recent blocks
- ``block.coinbase`` (``address``): current block miner's address
- ``block.difficulty`` (``uint``): current block difficulty
- ``block.gaslimit`` (``uint``): current block gaslimit
- ``block.number`` (``uint``): current block number
- ``block.timestamp`` (``uint``): current block timestamp
- ``msg.data`` (``bytes``): complete calldata
- ``msg.gas`` (``uint``): remaining gas
- ``msg.sender`` (``address``): sender of the message (current call)
- ``msg.value`` (``uint``): number of wei sent with the message
- ``now`` (``uint``): current block timestamp (alias for ``block.timestamp``)
- ``tx.gasprice`` (``uint``): gas price of the transaction
- ``tx.origin`` (``address``): sender of the transaction (full call chain)
- ``sha3(...) returns (bytes32)``: compute the Ethereum-SHA-3 (KECCAK-256) hash of the (tightly packed) arguments
- ``sha256(...) returns (bytes32)``: compute the SHA-256 hash of the (tightly packed) arguments
- ``ripemd160(...) returns (bytes20)``: compute the RIPEMD-160 hash of the (tightly packed) arguments
- ``ecrecover(bytes32 hash, uint8 v, bytes32 r, bytes32 s) returns (address)``: recover address associated with the public key from elliptic curve signature, return zero on error
- ``addmod(uint x, uint y, uint k) returns (uint)``: compute ``(x + y) % k`` where the addition is performed with arbitrary precision and does not wrap around at ``2**256``
- ``mulmod(uint x, uint y, uint k) returns (uint)``: compute ``(x * y) % k`` where the multiplication is performed with arbitrary precision and does not wrap around at ``2**256``
- ``this`` (current contract's type): the current contract, explicitly convertible to ``address``
- ``super``: the contract one level higher in the inheritance hierarchy
- ``selfdestruct(address recipient)``: destroy the current contract, sending its funds to the given address
- ``<address>.balance`` (``uint256``): balance of the address in Wei
- ``<address>.send(uint256 amount) returns (bool)``: send given amount of Wei to address, returns ``false`` on failure

.. index:: visibility, public, private, external, internal

Function Visibility Specifiers
==============================

::

    function myFunction() <visibility specifier> returns (bool) {
        return true;
    }

- ``public``: visible externally and internally (creates accessor function for storage/state variables)
- ``private``: only visible in the current contract
- ``external``: only visible externally (only for functions) - i.e. can only be message-called (via ``this.func``)
- ``internal``: only visible internally


.. index:: modifiers, constant, anonymous, indexed

Modifiers
=========

- ``constant`` for state variables: Disallows assignment (except initialisation), does not occupy storage slot.
- ``constant`` for functions: Disallows modification of state - this is not enforced yet.
- ``anonymous`` for events: Does not store event signature as topic.
- ``indexed`` for event parameters: Stores the parameter as topic.
