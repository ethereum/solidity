#################
Solidity Assembly
#################

.. index:: ! assembly, ! asm, ! evmasm

Solidity defines an assembly language that can also be used without Solidity.
This assembly language can also be used as "inline assembly" inside Solidity
source code. We start with describing how to use inline assembly and how it
differs from standalone assembly and then specify assembly itself.

TODO: Write about how scoping rules of inline assembly are a bit different
and the complications that arise when for example using internal functions
of libraries. Furhermore, write about the symbols defined by the compiler.

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

Standalone Assembly
===================

Grammar
-------

The assembly lexer follows the one defined by Solidity itself.

Whitespace is used to delimit tokens and it consists of the characters
Space, Tab and Linefeed. Comments as defined below, are interpreted in the
same way as Whitespace.
Furthermore, the following tokens exist:

TODO: escapes inside strings, decimal literals, hex literals, hex string literals

``OneLineComment := "//" [^\n]*`
``MultiLineComment := "/*" .*? "*/"`` 
 
``String := '"' [^"]* '"' | "'" [^']* "'"`` 
``Identifier := [_$a-zA-Z][_$a-zA-Z0-9]*``
``Opcodes :=
"add" | "addmod" | "address" | "and" | "balance" | "blockhash" | "byte" | "call" |
"callcode" | "calldatacopy" | "calldataload" | "calldatasize" | "caller" | "callvalue" |
"codecopy" | "codesize" | "coinbase" | "create" | "delegatecall" | "difficulty" |
"div" | "dup1" | "dup2" | "dup3" | "dup4" | "dup5" | "dup6" | "dup7" | "dup8" | "dup9" |
"dup10" | "dup11" | "dup12" | "dup13" | "dup14" | "dup15" | "dup16" | "eq" | "exp" |
"extcodecopy" | "extcodesize" | "gas" | "gaslimit" | "gasprice" | "gt" | "iszero" |
"jump" | "jumpi" | "log0" | "log1" | "log2" | "log3" | "log4" | "lt" | "mload" | "mod" |
"msize" | "mstore" | "mstore8" | "mul" | "mulmod" | "not" | "number" | "or" | "origin" |
"pc" | "pop" | "return" | "sdiv" | "selfdestruct" | "sgt" | "sha3" | "signextend" |
"sload" | "slt" | "smod" | "sstore" | "stop" | "sub" | "swap1" | "swap2" | "swap3" |
"swap4" | "swap5" | "swap6" | "swap7" | "swap8" | "swap9" | "swap10" | "swap11" |
"swap12" | "swap13" | "swap14" | "swap15" | "swap16" | "timestamp" | "xor"``

TODO: Define functional instruction, label, assignment, functional assignment,
variable declaration, ...


Syntax
------

Inline assembly parses comments, literals and identifiers exactly as Solidity, so you can use the
usual ``//`` and ``/* */`` comments. Inline assembly is initiated by ``assembly { ... }`` and inside
these curly braces, the following can be used (see the later sections for more details)

 - literals, i.e. ``0x123``, ``42`` or ``"abc"`` (strings up to 32 characters)
 - opcodes (in "instruction style"), e.g. ``mload sload dup1 sstore``, for a list see below
 - opcode in functional style, e.g. ``add(1, mlod(0))``
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
Note that the order of arguments can be seed to be reversed in non-functional style (explained below).
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
