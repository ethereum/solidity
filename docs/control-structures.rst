##################################
Expressions and Control Structures
##################################

.. index:: if, else, while, for, break, continue, return, switch, goto

Control Structures
===================

Most of the control structures from C/JavaScript are available in Solidity
except for ``switch`` and ``goto``. So
there is: ``if``, ``else``, ``while``, ``for``, ``break``, ``continue``, ``return``, ``? :``, with
the usual semantics known from C or JavaScript.

Parentheses can *not* be omitted for conditionals, but curly brances can be omitted
around single-statement bodies.

Note that there is no type conversion from non-boolean to boolean types as
there is in C and JavaScript, so ``if (1) { ... }`` is *not* valid Solidity.

.. index:: ! function;call, function;internal, function;external

.. _function-calls:

Function Calls
==============

Internal Function Calls
-----------------------

Functions of the current contract can be called directly ("internally"), also recursively, as seen in
this nonsensical example::

    contract C {
        function g(uint a) returns (uint ret) { return f(); }
        function f() returns (uint ret) { return g(7) + f(); }
    }

These function calls are translated into simple jumps inside the EVM. This has
the effect that the current memory is not cleared, i.e. passing memory references
to internally-called functions is very efficient. Only functions of the same
contract can be called internally.

External Function Calls
-----------------------

The expression ``this.g(8);`` is also a valid function call, but this time, the function
will be called "externally", via a message call and not directly via jumps.
Functions of other contracts have to be called externally. For an external call,
all function arguments have to be copied to memory.

When calling functions
of other contracts, the amount of Wei sent with the call and the gas can be specified::

    contract InfoFeed {
        function info() returns (uint ret) { return 42; }
    }


    contract Consumer {
        InfoFeed feed;
        function setFeed(address addr) { feed = InfoFeed(addr); }
        function callFeed() { feed.info.value(10).gas(800)(); }
    }

Note that the expression ``InfoFeed(addr)`` performs an explicit type conversion stating
that "we know that the type of the contract at the given address is ``InfoFeed``" and
this does not execute a constructor. We could also have used ``function setFeed(InfoFeed _feed) { feed = _feed; }`` directly.  Be careful about the fact that ``feed.info.value(10).gas(800)``
only (locally) sets the value and amount of gas sent with the function call and only the
parentheses at the end perform the actual call.

.. warning::
    Any interaction with another contract imposes a potential danger, especially
    if the source code of the contract is not known in advance. The current
    contract hands over control to the called contract and that may potentially
    do just about anything. Even if the called contract inherits from a known parent contract,
    the inheriting contract is only required to have a correct interface. The
    implementation of the contract, however, can be completely arbitrary and thus,
    pose a danger. In addition, be prepared in case it calls into other contracts of
    your system or even back into the calling contract before the first
    call returns. This means
    that the called contract can change state variables of the calling contract
    via its functions. Write your functions in a way that, for example, calls to
    external functions happen after any changes to state variables in your contract
    so your contract is not vulnerable to a recursive call exploit.

Named Calls and Anonymous Function Parameters
---------------------------------------------

Function call arguments can also be given by name, in any order,
if they are enclosed in ``{ }`` as can be seen in the following
example. The argument list has to coincide by name with the list of
parameters from the function declaration, but can be in arbitrary order.

::

    contract C {
        function f(uint key, uint value) { ... }

        function g() {
            // named arguments
            f({value: 2, key: 3});
        }
    }

Omitted Function Parameter Names
--------------------------------

The names of unused parameters (especially return parameters) can be omitted.
Those names will still be present on the stack, but they are inaccessible.

::

    contract C {
        // omitted name for parameter
        function func(uint k, uint) returns(uint) {
            return k;
        }
    }
    

.. index:: ! new, contracts;creating

.. _creating-contracts:

Creating Contracts via new
==========================

A contract can create a new contract using the ``new`` keyword. The full
code of the contract to be created has to be known and thus recursive
creation-dependencies are now possible.

::

    contract D {
        uint x;
        function D(uint a) {
            x = a;
        }
    }
    contract C {
        D d = new D(4); // will be executed as part of C's constructor

        function createD(uint arg) {
            D newD = new D(arg);
        }

        function createAndEndowD(uint arg, uint amount) {
            // Send ether along with the creation
            D newD = (new D).value(amount)(arg);
        }
    }

As seen in the example, it is possible to forward Ether to the creation,
but it is not possible to limit the amount of gas. If the creation fails
(due to out-of-stack, not enough balance or other problems), an exception
is thrown.

Order of Evaluation of Expressions
==================================

The evaluation order of expressions is not specified (more formally, the order
in which the children of one node in the expression tree are evaluated is not
specified, but they are of course evaluated before the node itself). It is only
guaranteed that statements are executed in order and short-circuiting for
boolean expressions is done. See :ref:`order` for more information.

.. index:: ! assignment

Assignment
==========

.. index:: ! assignment;destructuring

Destructuring Assignments and Returning Multiple Values
-------------------------------------------------------

Solidity internally allows tuple types, i.e. a list of objects of potentially different types whose size is a constant at compile-time. Those tuples can be used to return multiple values at the same time and also assign them to multiple variables (or LValues in general) at the same time::

    contract C {
        uint[] data;

        function f() returns (uint, bool, uint) {
            return (7, true, 2);
        }

        function g() {
            // Declares and assigns the variables. Specifying the type explicitly is not possible.
            var (x, b, y) = f();
            // Assigns to a pre-existing variable.
            (x, y) = (2, 7);
            // Common trick to swap values -- does not work for non-value storage types.
            (x, y) = (y, x);
            // Components can be left out (also for variable declarations).
            // If the tuple ends in an empty component,
            // the rest of the values are discarded.
            (data.length,) = f(); // Sets the length to 7
            // The same can be done on the left side.
            (,data[3]) = f(); // Sets data[3] to 2
            // Components can only be left out at the left-hand-side of assignments, with
            // one exception:
            (x,) = (1,);
            // (1,) is the only way to specify a 1-component tuple, because (1) is
            // equivalent to 1.
        }
    }

Complications for Arrays and Structs
------------------------------------

The semantics of assignment are a bit more complicated for non-value types like arrays and structs.
Assigning *to* a state variable always creates an independent copy. On the other hand, assigning to a local variable creates an independent copy only for elementary types, i.e. static types that fit into 32 bytes. If structs or arrays (including ``bytes`` and ``string``) are assigned from a state variable to a local variable, the local variable holds a reference to the original state variable. A second assignment to the local variable does not modify the state but only changes the reference. Assignments to members (or elements) of the local variable *do* change the state.

.. index:: ! scoping, declarations, default value

.. _default-value:

Scoping and Declarations
========================

A variable which is declared will have an initial default value whose byte-representation is all zeros.
The "default values" of variables are the typical "zero-state" of whatever the type is. For example, the default value for a ``bool``
is ``false``. The default value for the ``uint`` or ``int`` types is ``0``. For statically-sized arrays and ``bytes1`` to ``bytes32``, each individual
element will be initialized to the default value corresponding to its type. Finally, for dynamically-sized arrays, ``bytes``
and ``string``, the default value is an empty array or string.

A variable declared anywhere within a function will be in scope for the *entire function*, regardless of where it is declared.
This happens because Solidity inherits its scoping rules from JavaScript.
This is in contrast to many languages where variables are only scoped where they are declared until the end of the semantic block.
As a result, the following code is illegal and cause the compiler to throw an error, ``Identifier already declared``::

    contract ScopingErrors {
        function scoping() {
            uint i = 0;

            while (i++ < 1) {
                uint same1 = 0;
            }

            while (i++ < 2) {
                uint same1 = 0;// Illegal, second declaration of same1
            }
        }

        function minimalScoping() {
            {
                uint same2 = 0;
            }

            {
                uint same2 = 0;// Illegal, second declaration of same2
            }
        }

        function forLoopScoping() {
            for (uint same3 = 0; same3 < 1; same3++) {
            }

            for (uint same3 = 0; same3 < 1; same3++) {// Illegal, second declaration of same3
            }
        }
    }

In addition to this, if a variable is declared, it will be initialized at the beginning of the function to its default value.
As a result, the following code is legal, despite being poorly written::

    function foo() returns (uint) {
        // baz is implicitly initialized as 0
        uint bar = 5;
        if (true) {
            bar += baz;
        }
        else {
            uint baz = 10;// never executes
        }
        return bar;// returns 5
    }

.. index:: ! exception, ! throw

Exceptions
==========

There are some cases where exceptions are thrown automatically (see below). You can use the ``throw`` instruction to throw an exception manually. The effect of an exception is that the currently executing call is stopped and reverted (i.e. all changes to the state and balances are undone) and the exception is also "bubbled up" through Solidity function calls (exceptions are ``send`` and the low-level functions ``call``, ``delegatecall`` and ``callcode``, those return ``false`` in case of an exception).

Catching exceptions is not yet possible.

In the following example, we show how ``throw`` can be used to easily revert an Ether transfer and also how to check the return value of ``send``::

    contract Sharer {
        function sendHalf(address addr) returns (uint balance) {
            if (!addr.send(msg.value / 2))
                throw; // also reverts the transfer to Sharer
            return this.balance;
        }
    }

Currently, there are three situations, where exceptions happen automatically in Solidity:

1. If you access an array beyond its length (i.e. ``x[i]`` where ``i >= x.length``)
2. If a function called via a message call does not finish properly (i.e. it runs out of gas or throws an exception itself).
3. If a non-existent function on a library is called or Ether is sent to a library.

Internally, Solidity performs an "invalid jump" when an exception is thrown and thus causes the EVM to revert all changes made to the state. The reason for this is that there is no safe way to continue execution, because an expected effect did not occur. Because we want to retain the atomicity of transactions, the safest thing to do is to revert all changes and make the whole transaction (or at least call) without effect.

.. index:: ! assembly, ! asm, ! evmasm

Inline Assembly
===============

For more fine-grained control especially in order to enhance the language by writing libraries,
it is possible to interleave Solidity statements with inline assembly in a language close
to the one of the virtual machine. Due to the fact that the EVM is a stack machine, it is
often hard to address the correct stack slot and provide arguments to opcodes at the correct
point on the stack. Solidity's inline assembly tries to facilitate that and other issues
arising when writing manual assembly by the following features:

* functional-style opcodes: ``mul(1, add(2, 3))`` instead of ``push1 3 push1 2 add push1 1 mul``
* assembly-local variables: ``let x := add(2, 3)  let y := mload(0x40)  x := add(x, y)``
* access to external variables: ``function f(uint x) { assembly { x := sub(x, 1) } }``
* labels: ``let x := 10  repeat: x := sub(x, 1) jumpi(repeat, eq(x, 0))``

We now want to describe the inline assembly language in detail.

.. warning::
    Inline assembly is still a relatively new feature and might change if it does not prove useful,
    so please try to keep up to date.

Example
-------

The following example provides library code to access the code of another contract and
load it into a ``bytes`` variable. This is not possible at all with "plain Solidity" and the
idea is that assembly libraries will be used to enhance the language in such ways.

.. code::

    library GetCode {
        function at(address _addr) returns (bytes o_code) {
            assembly {
                // retrieve the size of the code, this needs assembly
                let size := extcodesize(_addr)
                // allocate output byte array - this could also be done without assembly
                // by using o_code = new bytes(size)
                o_code := mload(0x40)
                // new "memory end" including padding
                mstore(0x40, add(o_code, and(add(add(size, 0x20), 0x1f), not(0x1f))))
                // store length in memory
                mstore(o_code, size)
                // actually retrieve the code, this needs assembly
                extcodecopy(_addr, add(o_code, 0x20), 0, size)
            }
        }
    }

Inline assembly could also be beneficial in cases where the optimizer fails to produce
efficient code. Please be aware that assembly is much more difficult to write because
the compiler does not perform checks, so you should use it for complex things only if
you really know what you are doing.

.. code::

    library VectorSum {
        // This function is less efficient because the optimizer currently fails to
        // remove the bounds checks in array access.
        function sumSolidity(uint[] _data) returns (uint o_sum) {
            for (uint i = 0; i < _data.length; ++i)
                o_sum += _data[i];
        }

        // We know that we only access the array in bounds, so we can avoid the check.
        // 0x20 needs to be added to an array because the first slot contains the
        // array length.
        function sumAsm(uint[] _data) returns (uint o_sum) {
            for (uint i = 0; i < _data.length; ++i) {
                assembly {
                    o_sum := mload(add(add(_data, 0x20), i))
                }
            }
        }
    }

Syntax
------

Inline assembly parses comments, literals and identifiers exactly as Solidity, so you can use the
usual ``//`` and ``/* */`` comments. Inline assembly is initiated by ``assembly { ... }`` and inside
these curly braces, the following can be used (see the later sections for more details)

 - literals, e.g. ``0x123``, ``42`` or ``"abc"`` (strings up to 32 characters)
 - opcodes (in "instruction style"), e.g. ``mload sload dup1 sstore``, for a list see below
 - opcodes in functional style, e.g. ``add(1, mlod(0))``
 - labels, e.g. ``name:``
 - variable declarations, e.g. ``let x := 7`` or ``let x := add(y, 3)``
 - identifiers (externals, labels or assembly-local variables), e.g. ``jump(name)``, ``3 x add``
 - assignments (in "instruction style"), e.g. ``3 =: x``
 - assignments in functional style, e.g. ``x := add(y, 3)``
 - blocks where local variables are scoped inside, e.g. ``{ let x := 3 { let y := add(x, 1) } }``

Opcodes
-------

This document does not want to be a full description of the Ethereum virtual machine, but the
following list can be used as a reference of its opcodes.

If an opcode takes arguments (always from the top of the stack), they are given in parentheses.
Note that the order of arguments can be seen as being reversed compared to the instructional style (explained below).
Opcodes marked with ``-`` do not push an item onto the stack, those marked with ``*`` are
special and all others push exactly one item onte the stack.

In the following, ``mem[a...b)`` signifies the bytes of memory starting at position ``a`` up to
(excluding) position ``b`` and ``storage[p]`` signifies the storage contents at position ``p``.

The opcodes ``pushi`` and ``jumpdest`` cannot be used directly.

+-------------------------+------+-----------------------------------------------------------------+
| stop                    + `-`  | stop execution, identical to return(0,0)                        |
+-------------------------+------+-----------------------------------------------------------------+
| add(x, y)               |      | x + y                                                           |
+-------------------------+------+-----------------------------------------------------------------+
| sub(x, y)               |      | x - y                                                           |
+-------------------------+------+-----------------------------------------------------------------+
| mul(x, y)               |      | x * y                                                           |
+-------------------------+------+-----------------------------------------------------------------+
| div(x, y)               |      | x / y                                                           |
+-------------------------+------+-----------------------------------------------------------------+
| sdiv(x, y)              |      | x / y, for signed numbers in two's complement                   |
+-------------------------+------+-----------------------------------------------------------------+
| mod(x, y)               |      | x % y                                                           |
+-------------------------+------+-----------------------------------------------------------------+
| smod(x, y)              |      | x % y, for signed numbers in two's complement                   |
+-------------------------+------+-----------------------------------------------------------------+
| exp(x, y)               |      | x to the power of y                                             |
+-------------------------+------+-----------------------------------------------------------------+
| not(x)                  |      | ~x, every bit of x is negated                                   |
+-------------------------+------+-----------------------------------------------------------------+
| lt(x, y)                |      | 1 if x < y, 0 otherwise                                         |
+-------------------------+------+-----------------------------------------------------------------+
| gt(x, y)                |      | 1 if x > y, 0 otherwise                                         |
+-------------------------+------+-----------------------------------------------------------------+
| slt(x, y)               |      | 1 if x < y, 0 otherwise, for signed numbers in two's complement |
+-------------------------+------+-----------------------------------------------------------------+
| sgt(x, y)               |      | 1 if x > y, 0 otherwise, for signed numbers in two's complement |
+-------------------------+------+-----------------------------------------------------------------+
| eq(x, y)                |      | 1 if x == y, 0 otherwise                                        |
+-------------------------+------+-----------------------------------------------------------------+
| iszero(x)               |      | 1 if x == 0, 0 otherwise                                        |
+-------------------------+------+-----------------------------------------------------------------+
| and(x, y)               |      | bitwise and of x and y                                          |
+-------------------------+------+-----------------------------------------------------------------+
| or(x, y)                |      | bitwise or of x and y                                           |
+-------------------------+------+-----------------------------------------------------------------+
| xor(x, y)               |      | bitwise xor of x and y                                          |
+-------------------------+------+-----------------------------------------------------------------+
| byte(n, x)              |      | nth byte of x, where the most significant byte is the 0th byte  |
+-------------------------+------+-----------------------------------------------------------------+
| addmod(x, y, m)         |      | (x + y) % m with arbitrary precision arithmetics                |
+-------------------------+------+-----------------------------------------------------------------+
| mulmod(x, y, m)         |      | (x * y) % m with arbitrary precision arithmetics                |
+-------------------------+------+-----------------------------------------------------------------+
| signextend(i, x)        |      | sign extend from (i*8+7)th bit counting from least significant  |
+-------------------------+------+-----------------------------------------------------------------+
| sha3(p, n)              |      | keccak(mem[p...(p+n)))                                          |
+-------------------------+------+-----------------------------------------------------------------+
| jump(label)             | `-`  | jump to label / code position                                   |
+-------------------------+------+-----------------------------------------------------------------+
| jumpi(label, cond)      | `-`  | jump to label if cond is nonzero                                |
+-------------------------+------+-----------------------------------------------------------------+
| pc                      |      | current position in code                                        |
+-------------------------+------+-----------------------------------------------------------------+
| pop                     | `*`  | remove topmost stack slot                                       |
+-------------------------+------+-----------------------------------------------------------------+
| dup1 ... dup16          |      | copy ith stack slot to the top (counting from top)              |
+-------------------------+------+-----------------------------------------------------------------+
| swap1 ... swap16        | `*`  | swap topmost and ith stack slot below it                        |
+-------------------------+------+-----------------------------------------------------------------+
| mload(p)                |      | mem[p..(p+32))                                                  |
+-------------------------+------+-----------------------------------------------------------------+
| mstore(p, v)            | `-`  | mem[p..(p+32)) := v                                             |
+-------------------------+------+-----------------------------------------------------------------+
| mstore8(p, v)           | `-`  | mem[p] := v & 0xff    - only modifies a single byte             |
+-------------------------+------+-----------------------------------------------------------------+
| sload(p)                |      | storage[p]                                                      |
+-------------------------+------+-----------------------------------------------------------------+
| sstore(p, v)            | `-`  | storage[p] := v                                                 |
+-------------------------+------+-----------------------------------------------------------------+
| msize                   |      | size of memory, i.e. largest accessed memory index              |
+-------------------------+------+-----------------------------------------------------------------+
| gas                     |      | gas still available to execution                                |
+-------------------------+------+-----------------------------------------------------------------+
| address                 |      | address of the current contract / execution context             |
+-------------------------+------+-----------------------------------------------------------------+
| balance(a)              |      | wei balance at address a                                        |
+-------------------------+------+-----------------------------------------------------------------+
| caller                  |      | call sender (excluding delegatecall)                            |
+-------------------------+------+-----------------------------------------------------------------+
| callvalue               |      | wei sent together with the current call                         |
+-------------------------+------+-----------------------------------------------------------------+
| calldataload(p)         |      | call data starting from position p (32 bytes)                   |
+-------------------------+------+-----------------------------------------------------------------+
| calldatasize            |      | size of call data in bytes                                      |
+-------------------------+------+-----------------------------------------------------------------+
| calldatacopy(t, f, s)   | `-`  | copy s bytes from calldata at position f to mem at position t   |
+-------------------------+------+-----------------------------------------------------------------+
| codesize                |      | size of the code of the current contract / execution context    |
+-------------------------+------+-----------------------------------------------------------------+
| codecopy(t, f, s)       | `-`  | copy s bytes from code at position f to mem at position t       |
+-------------------------+------+-----------------------------------------------------------------+
| extcodesize(a)          |      | size of the code at address a                                   |
+-------------------------+------+-----------------------------------------------------------------+
| extcodecopy(a, t, f, s) | `-`  | like codecopy(t, f, s) but take code at address a               |
+-------------------------+------+-----------------------------------------------------------------+
| create(v, p, s)         |      | create new contract with code mem[p..(p+s)) and send v wei      |
|                         |      | and return the new address                                      |
+-------------------------+------+-----------------------------------------------------------------+
| call(g, a, v, in,       |      | call contract at address a with input mem[in..(in+insize)]      |
| insize, out, outsize)   |      | providing g gas and v wei and output area                       |
|                         |      | mem[out..(out+outsize)] returting 1 on error (out of gas)       |
+-------------------------+------+-----------------------------------------------------------------+
| callcode(g, a, v, in,   |      | identical to call but only use the code from a and stay         |
| insize, out, outsize)   |      | in the context of the current contract otherwise                |
+-------------------------+------+-----------------------------------------------------------------+
| delegatecall(g, a, in,  |      | identical to callcode but also keep ``caller``                  |
| insize, out, outsize)   |      | and ``callvalue``                                               |
+-------------------------+------+-----------------------------------------------------------------+
| return(p, s)            | `*`  | end execution, return data mem[p..(p+s))                        |
+-------------------------+------+-----------------------------------------------------------------+
| selfdestruct(a)         | `*`  | end execution, destroy current contract and send funds to a     |
+-------------------------+------+-----------------------------------------------------------------+
| log0(p, s)              | `-`  | log without topics and data mem[p..(p+s))                       |
+-------------------------+------+-----------------------------------------------------------------+
| log1(p, s, t1)          | `-`  | log with topic t1 and data mem[p..(p+s))                        |
+-------------------------+------+-----------------------------------------------------------------+
| log2(p, s, t1, t2)      | `-`  | log with topics t1, t2 and data mem[p..(p+s))                   |
+-------------------------+------+-----------------------------------------------------------------+
| log3(p, s, t1, t2, t3)  | `-`  | log with topics t1, t2, t3 and data mem[p..(p+s))               |
+-------------------------+------+-----------------------------------------------------------------+
| log4(p, s, t1, t2, t3,  | `-`  | log with topics t1, t2, t3, t4 and data mem[p..(p+s))           |
| t4)                     |      |                                                                 |
+-------------------------+------+-----------------------------------------------------------------+
| origin                  |      | transaction sender                                              |
+-------------------------+------+-----------------------------------------------------------------+
| gasprice                |      | gas price of the transaction                                    |
+-------------------------+------+-----------------------------------------------------------------+
| blockhash(b)            |      | hash of block nr b - only for last 256 blocks excluding current |
+-------------------------+------+-----------------------------------------------------------------+
| coinbase                |      | current mining beneficiary                                      |
+-------------------------+------+-----------------------------------------------------------------+
| timestamp               |      | timestamp of the current block in seconds since the epoch       |
+-------------------------+------+-----------------------------------------------------------------+
| number                  |      | current block number                                            |
+-------------------------+------+-----------------------------------------------------------------+
| difficulty              |      | difficulty of the current block                                 |
+-------------------------+------+-----------------------------------------------------------------+
| gaslimit                |      | block gas limit of the current block                            |
+-------------------------+------+-----------------------------------------------------------------+

Literals
--------

You can use integer constants by typing them in decimal or hexadecimal notation and an
appropriate ``PUSHi`` instruction will automatically be generated. The following creates code
to add 2 and 3 resulting in 5 and then computes the bitwise and with the string "abc".
Strings are stored left-aligned and cannot be longer than 32 bytes.

.. code::

    assembly { 2 3 add "abc" and }

Functional Style
-----------------

You can type opcode after opcode in the same way they will end up in bytecode. For example
adding ``3`` to the contents in memory at position ``0x80`` would be

.. code::

    3 0x80 mload add 0x80 mstore

As it is often hard to see what the actual arguments for certain opcodes are,
Solidity inline assembly also provides a "functional style" notation where the same code
would be written as follows

.. code::

    mstore(0x80, add(mload(0x80), 3))

Functional style and instructional style can be mixed, but any opcode inside a
functional style expression has to return exactly one stack slot (most of the opcodes do).

Note that the order of arguments is reversed in functional-style as opposed to the instruction-style
way. If you use functional-style, the first argument will end up on the stack top.


Access to External Variables and Functions
------------------------------------------

Solidity variables and other identifiers can be accessed by simply using their name.
For storage and memory variables, this will push the address and not the value onto the
stack. Also note that non-struct and non-array storage variable addresses occupy two slots
on the stack: One for the address and one for the byte offset inside the storage slot.
In assignments (see below), we can even use local Solidity variables to assign to.

Functions external to inline assembly can also be accessed: The assembly will
push their entry label (with virtual function resolution applied). The calling semantics
in solidity are:

 - the caller pushes return label, arg1, arg2, ..., argn
 - the call returns with ret1, ret2, ..., retn

This feature is still a bit cumbersome to use, because the stack offset essentially
changes during the call, and thus references to local variables will be wrong.
It is planned that the stack height changes can be specified in inline assembly.

.. code::

    contract C {
        uint b;
        function f(uint x) returns (uint r) {
            assembly {
                b pop // remove the offset, we know it is zero
                sload
                x
                mul
                =: r  // assign to return variable r
            }
        }
    }

Labels
------

Another problem in EVM assembly is that ``jump`` and ``jumpi`` use absolute addresses
which can change easily. Solidity inline assembly provides labels to make the use of
jumps easier. The following code computes an element in the Fibonacci series.

.. code::

    {
        let n := calldataload(4)
        let a := 1
        let b := a
    loop:
        jumpi(loopend, eq(n, 0))
        a add swap1
        n := sub(n, 1)
        jump(loop)
    loopend:
        mstore(0, a)
        return(0, 0x20)
    }

Please note that automatically accessing stack variables can only work if the
assembler knows the current stack height. This fails to work if the jump source
and target have different stack heights. It is still fine to use such jumps,
you should just not access any stack variables (even assembly variables) in that case.

Furthermore, the stack height analyser goes through the code opcode by opcode
(and not according to control flow), so in the following case, the assembler
will have a wrong impression about the stack height at label ``two``:

.. code::

    {
        jump(two)
        one:
            // Here the stack height is 1 (because we pushed 7),
            // but the assembler thinks it is 0 because it reads
            // from top to bottom.
            // Accessing stack variables here will lead to errors.
            jump(three)
        two:
            7 // push something onto the stack
            jump(one)
        three:
    }


Declaring Assembly-Local Variables
----------------------------------

You can use the ``let`` keyword to declare variables that are only visible in
inline assembly and actually only in the current ``{...}``-block. What happens
is that the ``let`` instruction will create a new stack slot that is reserved
for the variable and automatically removed again when the end of the block
is reached. You need to provide an initial value for the variable which can
be just ``0``, but it can also be a complex functional-style expression.

.. code::

    contract C {
        function f(uint x) returns (uint b) {
            assembly {
                let v := add(x, 1)
                mstore(0x80, v)
                {
                    let y := add(sload(v), 1)
                    b := y
                } // y is "deallocated" here
                b := add(b, v)
            } // v is "deallocated" here
        }
    }


Assignments
-----------

Assignments are possible to assembly-local variables and to function-local
variables. Take care that when you assign to variables that point to
memory or storage, you will only change the pointer and not the data.

There are two kinds of assignments: Functional-style and instruction-style.
For functional-style assignments (``variable := value``), you need to provide a value in a
functional-style expression that results in exactly one stack value
and for instruction-style (``=: variable``), the value is just taken from the stack top.
For both ways, the colon points to the name of the variable.

.. code::

    assembly {
        let v := 0 // functional-style assignment as part of variable declaration
        let g := add(v, 2)
        sload(10)
        =: v // instruction style assignment, puts the result of sload(10) into v
    }


Things to Avoid
---------------

Inline assembly might have a quite high-level look, but it actually is extremely
low-level. The only thing the assembler does for you is re-arranging
functional-style opcodes, managing jump labels, counting stack height for
variable access and removing stack slots for assembly-local variables when the end
of their block is reached. Especially for those two last cases, it is important
to know that the assembler only counts stack height from top to bottom, not
necessarily following control flow. Furthermore, operations like swap will only
swap the contents of the stack but not the location of variables.

Conventions in Solidity
-----------------------

In contrast to EVM assembly, Solidity knows types which are narrower than 256 bits,
e.g. ``uint24``. In order to make them more efficient, most arithmetic operations just
treat them as 256 bit numbers and the higher-order bits are only cleaned at the
point where it is necessary, i.e. just shortly before they are written to memory
or before comparisons are performed. This means that if you access such a variable
from within inline assembly, you might have to manually clean the higher order bits
first.

Solidity manages memory in a very simple way: There is a "free memory pointer"
at position ``0x40`` in memory. If you want to allocate memory, just use the memory
from that point on and update the pointer accordingly.

Elements in memory arrays in Solidity always occupy multiples of 32 bytes (yes, this is
even true for ``byte[]``, but not for ``bytes`` and ``string``). Multi-dimensional memory
arrays are pointers to memory arrays. The length of a dynamic array is stored at the
first slot of the array and then only the array elements follow.

.. warning::
    Statically-sized memory arrays do not have a length field, but it will be added soon
    to allow better convertibility between statically- and dynamically-sized arrays, so
    please do not rely on that.
