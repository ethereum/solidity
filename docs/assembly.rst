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
of libraries. Furthermore, write about the symbols defined by the compiler.

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
* loops: ``for { let i := 0 } lt(i, x) { i := add(i, 1) } { y := mul(2, y) }``
* switch statements: ``switch x case 0: { y := mul(x, 2) } default: { y := 0 }``
* function calls: ``function f(x) -> y { switch x case 0: { y := 1 } default: { y := mul(x, f(sub(x, 1))) }   }``

.. note::
    Of the above, loops, function calls and switch statements are not yet implemented.

We now want to describe the inline assembly language in detail.

.. warning::
    Inline assembly is a way to access the Ethereum Virtual Machine
    at a low level. This discards several important safety
    features of Solidity.

Example
-------

The following example provides library code to access the code of another contract and
load it into a ``bytes`` variable. This is not possible at all with "plain Solidity" and the
idea is that assembly libraries will be used to enhance the language in such ways.

.. code::

    pragma solidity ^0.4.0;

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

    pragma solidity ^0.4.0;

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
                    o_sum := mload(add(add(_data, 0x20), mul(i, 0x20)))
                }
            }
        }
    }


Syntax
------

Assembly parses comments, literals and identifiers exactly as Solidity, so you can use the
usual ``//`` and ``/* */`` comments. Inline assembly is marked by ``assembly { ... }`` and inside
these curly braces, the following can be used (see the later sections for more details)

 - literals, i.e. ``0x123``, ``42`` or ``"abc"`` (strings up to 32 characters)
 - opcodes (in "instruction style"), e.g. ``mload sload dup1 sstore``, for a list see below
 - opcodes in functional style, e.g. ``add(1, mlod(0))``
 - labels, e.g. ``name:``
 - variable declarations, e.g. ``let x := 7`` or ``let x := add(y, 3)``
 - identifiers (labels or assembly-local variables and externals if used as inline assembly), e.g. ``jump(name)``, ``3 x add``
 - assignments (in "instruction style"), e.g. ``3 =: x``
 - assignments in functional style, e.g. ``x := add(y, 3)``
 - blocks where local variables are scoped inside, e.g. ``{ let x := 3 { let y := add(x, 1) } }``

Opcodes
-------

This document does not want to be a full description of the Ethereum virtual machine, but the
following list can be used as a reference of its opcodes.

If an opcode takes arguments (always from the top of the stack), they are given in parentheses.
Note that the order of arguments can be seen to be reversed in non-functional style (explained below).
Opcodes marked with ``-`` do not push an item onto the stack, those marked with ``*`` are
special and all others push exactly one item onte the stack.

In the following, ``mem[a...b)`` signifies the bytes of memory starting at position ``a`` up to
(excluding) position ``b`` and ``storage[p]`` signifies the storage contents at position ``p``.

The opcodes ``pushi`` and ``jumpdest`` cannot be used directly.

In the grammar, opcodes are represented as pre-defined identifiers.

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
| pop(x)                  | `-`  | remove the element pushed by x                                  |
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
| call(g, a, v, in,       |      | call contract at address a with input mem[in..(in+insize))      |
| insize, out, outsize)   |      | providing g gas and v wei and output area                       |
|                         |      | mem[out..(out+outsize)) returning 0 on error (eg. out of gas)   |
|                         |      | and 1 on success                                                |
+-------------------------+------+-----------------------------------------------------------------+
| callcode(g, a, v, in,   |      | identical to `call` but only use the code from a and stay       |
| insize, out, outsize)   |      | in the context of the current contract otherwise                |
+-------------------------+------+-----------------------------------------------------------------+
| delegatecall(g, a, in,  |      | identical to `callcode` but also keep ``caller``                |
| insize, out, outsize)   |      | and ``callvalue``                                               |
+-------------------------+------+-----------------------------------------------------------------+
| return(p, s)            | `-`  | end execution, return data mem[p..(p+s))                        |
+-------------------------+------+-----------------------------------------------------------------+
| revert(p, s)            | `-`  | end execution, revert state changes, return data mem[p..(p+s))  |
+-------------------------+------+-----------------------------------------------------------------+
| selfdestruct(a)         | `-`  | end execution, destroy current contract and send funds to a     |
+-------------------------+------+-----------------------------------------------------------------+
| invalid                 | `-`  | end execution with invalid instruction                          |
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
For memory variables, this will push the address and not the value onto the
stack. Storage variables are different: Values in storage might not occupy a
full storage slot, so their "address" is composed of a slot and a byte-offset
inside that slot. To retrieve the slot pointed to by the variable ``x``, you
used ``x_slot`` and to retrieve the byte-offset you used ``x_offset``.

In assignments (see below), we can even use local Solidity variables to assign to.

Functions external to inline assembly can also be accessed: The assembly will
push their entry label (with virtual function resolution applied). The calling semantics
in solidity are:

 - the caller pushes return label, arg1, arg2, ..., argn
 - the call returns with ret1, ret2, ..., retm

This feature is still a bit cumbersome to use, because the stack offset essentially
changes during the call, and thus references to local variables will be wrong.

.. code::

    pragma solidity ^0.4.11;

    contract C {
        uint b;
        function f(uint x) returns (uint r) {
            assembly {
                r := mul(x, sload(b_slot)) // ignore the offset, we know it is zero
            }
        }
    }

Labels
------

Another problem in EVM assembly is that ``jump`` and ``jumpi`` use absolute addresses
which can change easily. Solidity inline assembly provides labels to make the use of
jumps easier. Note that labels are a low-level feature and it is possible to write
efficient assembly without labels, just using assembly functions, loops and switch instructions
(see below). The following code computes an element in the Fibonacci series.

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
and target have different stack heights. It is still fine to use such jumps, but
you should just not access any stack variables (even assembly variables) in that case.

Furthermore, the stack height analyser goes through the code opcode by opcode
(and not according to control flow), so in the following case, the assembler
will have a wrong impression about the stack height at label ``two``:

.. code::

    {
        let x := 8
        jump(two)
        one:
            // Here the stack height is 2 (because we pushed x and 7),
            // but the assembler thinks it is 1 because it reads
            // from top to bottom.
            // Accessing the stack variable x here will lead to errors.
            x := 9
            jump(three)
        two:
            7 // push something onto the stack
            jump(one)
        three:
    }

This problem can be fixed by manually adjusting the stack height for the
assembler - you can provide a stack height delta that is added
to the stack height just prior to the label.
Note that you will not have to care about these things if you just use
loops and assembly-level functions.

As an example how this can be done in extreme cases, please see the following.

.. code::

    {
        let x := 8
        jump(two)
        0 // This code is unreachable but will adjust the stack height correctly
        one:
            x := 9 // Now x can be accessed properly.
            jump(three)
            pop // Similar negative correction.
        two:
            7 // push something onto the stack
            jump(one)
        three:
        pop // We have to pop the manually pushed value here again.
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

    pragma solidity ^0.4.0;

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

There are two kinds of assignments: functional-style and instruction-style.
For functional-style assignments (``variable := value``), you need to provide a value in a
functional-style expression that results in exactly one stack value
and for instruction-style (``=: variable``), the value is just taken from the stack top.
For both ways, the colon points to the name of the variable. The assignment
is performed by replacing the variable's value on the stack by the new value.

.. code::

    assembly {
        let v := 0 // functional-style assignment as part of variable declaration
        let g := add(v, 2)
        sload(10)
        =: v // instruction style assignment, puts the result of sload(10) into v
    }

Switch
------

.. note::
    Switch is not yet implemented.

You can use a switch statement as a very basic version of "if/else".
It takes the value of an expression and compares it to several constants.
The branch corresponding to the matching constant is taken. Contrary to the
error-prone behaviour of some programming languages, control flow does
not continue from one case to the next. There can be a fallback or default
case called ``default``.

.. code::

    assembly {
        let x := 0
        switch calldataload(4)
        case 0: {
            x := calldataload(0x24)
        }
        default: {
            x := calldataload(0x44)
        }
        sstore(0, div(x, 2))
    }

The list of cases does not require curly braces, but the body of a
case does require them.

Loops
-----

.. note::
    Loops are not yet implemented.

Assembly supports a simple for-style loop. For-style loops have
a header containing an initializing part, a condition and a post-iteration
part. The condition has to be a functional-style expression, while
the other two can also be blocks. If the initializing part is a block that
declares any variables, the scope of these variables is extended into the
body (including the condition and the post-iteration part).

The following example computes the sum of an area in memory.

.. code::

    assembly {
        let x := 0
        for { let i := 0 } lt(i, 0x100) { i := add(i, 0x20) } {
            x := add(x, mload(i))
        }
    }

Functions
---------

.. note::
    Functions are not yet implemented.

Assembly allows the definition of low-level functions. These take their
arguments (and a return PC) from the stack and also put the results onto the
stack. Calling a function looks the same way as executing a functional-style
opcode.

Functions can be defined anywhere and are visible in the block they are
declared in. Inside a function, you cannot access local variables
defined outside of that function. There is no explicit ``return``
statement.

If you call a function that returns multiple values, you have to assign
them to a tuple using ``(a, b) := f(x)`` or ``let (a, b) := f(x)``.

The following example implements the power function by square-and-multiply.

.. code::

    assembly {
        function power(base, exponent) -> result {
            switch exponent
            0: { result := 1 }
            1: { result := base }
            default: {
                result := power(mul(base, base), div(exponent, 2))
                switch mod(exponent, 2)
                    1: { result := mul(base, result) }
            }
        }
    }

Things to Avoid
---------------

Inline assembly might have a quite high-level look, but it actually is extremely
low-level. Function calls, loops and switches are converted by simple
rewriting rules and after that, the only thing the assembler does for you is re-arranging
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
treat them as 256-bit numbers and the higher-order bits are only cleaned at the
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


Standalone Assembly
===================

The assembly language described as inline assembly above can also be used
standalone and in fact, the plan is to use it as an intermediate language
for the Solidity compiler. In this form, it tries to achieve several goals:

1. Programs written in it should be readable, even if the code is generated by a compiler from Solidity.
2. The translation from assembly to bytecode should contain as few "surprises" as possible.
3. Control flow should be easy to detect to help in formal verification and optimization.

In order to achieve the first and last goal, assembly provides high-level constructs
like ``for`` loops, ``switch`` statements and function calls. It should be possible
to write assembly programs that do not make use of explicit ``SWAP``, ``DUP``,
``JUMP`` and ``JUMPI`` statements, because the first two obfuscate the data flow
and the last two obfuscate control flow. Furthermore, functional statements of
the form ``mul(add(x, y), 7)`` are preferred over pure opcode statements like
``7 y x add mul`` because in the first form, it is much easier to see which
operand is used for which opcode.

The second goal is achieved by introducing a desugaring phase that only removes
the higher level constructs in a very regular way and still allows inspecting
the generated low-level assembly code. The only non-local operation performed
by the assembler is name lookup of user-defined identifiers (functions, variables, ...),
which follow very simple and regular scoping rules and cleanup of local variables from the stack.

Scoping: An identifier that is declared (label, variable, function, assembly)
is only visible in the block where it was declared (including nested blocks
inside the current block). It is not legal to access local variables across
function borders, even if they would be in scope. Shadowing is not allowed.
Local variables cannot be accessed before they were declared, but labels,
functions and assemblies can. Assemblies are special blocks that are used
for e.g. returning runtime code or creating contracts. No identifier from an
outer assembly is visible in a sub-assembly.

If control flow passes over the end of a block, pop instructions are inserted
that match the number of local variables declared in that block.
Whenever a local variable is referenced, the code generator needs
to know its current relative position in the stack and thus it needs to
keep track of the current so-called stack height. Since all local variables
are removed at the end of a block, the stack height before and after the block
should be the same. If this is not the case, a warning is issued.

Why do we use higher-level constructs like ``switch``, ``for`` and functions:

Using ``switch``, ``for`` and functions, it should be possible to write
complex code without using ``jump`` or ``jumpi`` manually. This makes it much
easier to analyze the control flow, which allows for improved formal
verification and optimization.

Furthermore, if manual jumps are allowed, computing the stack height is rather complicated.
The position of all local variables on the stack needs to be known, otherwise
neither references to local variables nor removing local variables automatically
from the stack at the end of a block will work properly. The desugaring
mechanism correctly inserts operations at unreachable blocks that adjust the
stack height properly in case of jumps that do not have a continuing control flow.

Example:

We will follow an example compilation from Solidity to desugared assembly.
We consider the runtime bytecode of the following Solidity program::

    contract C {
      function f(uint x) returns (uint y) {
        y = 1;
        for (uint i = 0; i < x; i++)
          y = 2 * y;
      }
    }

The following assembly will be generated::

    {
      mstore(0x40, 0x60) // store the "free memory pointer"
      // function dispatcher
      switch div(calldataload(0), exp(2, 226))
      case 0xb3de648b: {
        let (r) = f(calldataload(4))
        let ret := $allocate(0x20)
        mstore(ret, r)
        return(ret, 0x20)
      }
      default: { revert(0, 0) }
      // memory allocator
      function $allocate(size) -> pos {
        pos := mload(0x40)
        mstore(0x40, add(pos, size))
      }
      // the contract function
      function f(x) -> y {
        y := 1
        for { let i := 0 } lt(i, x) { i := add(i, 1) } {
          y := mul(2, y)
        }
      }
    }

After the desugaring phase it looks as follows::

    {
      mstore(0x40, 0x60)
      {
        let $0 := div(calldataload(0), exp(2, 226))
        jumpi($case1, eq($0, 0xb3de648b))
        jump($caseDefault)
        $case1:
        {
          // the function call - we put return label and arguments on the stack
          $ret1 calldataload(4) jump(f)
          // This is unreachable code. Opcodes are added that mirror the
          // effect of the function on the stack height: Arguments are
          // removed and return values are introduced.
          pop pop
          let r := 0
          $ret1: // the actual return point
          $ret2 0x20 jump($allocate)
          pop pop let ret := 0
          $ret2:
          mstore(ret, r)
          return(ret, 0x20)
          // although it is useless, the jump is automatically inserted,
          // since the desugaring process is a purely syntactic operation that
          // does not analyze control-flow
          jump($endswitch)
        }
        $caseDefault:
        {
          revert(0, 0)
          jump($endswitch)
        }
        $endswitch:
      }
      jump($afterFunction)
      allocate:
      {
        // we jump over the unreachable code that introduces the function arguments
        jump($start)
        let $retpos := 0 let size := 0
        $start:
        // output variables live in the same scope as the arguments and is
        // actually allocated.
        let pos := 0
        {
          pos := mload(0x40)
          mstore(0x40, add(pos, size))
        }
        // This code replaces the arguments by the return values and jumps back.
        swap1 pop swap1 jump
        // Again unreachable code that corrects stack height.
        0 0
      }
      f:
      {
        jump($start)
        let $retpos := 0 let x := 0
        $start:
        let y := 0
        {
          let i := 0
          $for_begin:
          jumpi($for_end, iszero(lt(i, x)))
          {
            y := mul(2, y)
          }
          $for_continue:
          { i := add(i, 1) }
          jump($for_begin)
          $for_end:
        } // Here, a pop instruction will be inserted for i
        swap1 pop swap1 jump
        0 0
      }
      $afterFunction:
      stop
    }


Assembly happens in four stages:

1. Parsing
2. Desugaring (removes switch, for and functions)
3. Opcode stream generation
4. Bytecode generation

We will specify steps one to three in a pseudo-formal way. More formal
specifications will follow.


Parsing / Grammar
-----------------

The tasks of the parser are the following:

- Turn the byte stream into a token stream, discarding C++-style comments
  (a special comment exists for source references, but we will not explain it here).
- Turn the token stream into an AST according to the grammar below
- Register identifiers with the block they are defined in (annotation to the
  AST node) and note from which point on, variables can be accessed.

The assembly lexer follows the one defined by Solidity itself.

Whitespace is used to delimit tokens and it consists of the characters
Space, Tab and Linefeed. Comments are regular JavaScript/C++ comments and
are interpreted in the same way as Whitespace.

Grammar::

    AssemblyBlock = '{' AssemblyItem* '}'
    AssemblyItem =
        Identifier |
        AssemblyBlock |
        FunctionalAssemblyExpression |
        AssemblyLocalDefinition |
        FunctionalAssemblyAssignment |
        AssemblyAssignment |
        LabelDefinition |
        AssemblySwitch |
        AssemblyFunctionDefinition |
        AssemblyFor |
        'break' | 'continue' |
        SubAssembly | 'dataSize' '(' Identifier ')' |
        LinkerSymbol |
        'errorLabel' | 'bytecodeSize' |
        NumberLiteral | StringLiteral | HexLiteral
    Identifier = [a-zA-Z_$] [a-zA-Z_0-9]*
    FunctionalAssemblyExpression = Identifier '(' ( AssemblyItem ( ',' AssemblyItem )* )? ')'
    AssemblyLocalDefinition = 'let' IdentifierOrList ':=' FunctionalAssemblyExpression
    FunctionalAssemblyAssignment = IdentifierOrList ':=' FunctionalAssemblyExpression
    IdentifierOrList = Identifier | '(' IdentifierList ')'
    IdentifierList = Identifier ( ',' Identifier)*
    AssemblyAssignment = '=:' Identifier
    LabelDefinition = Identifier ':'
    AssemblySwitch = 'switch' FunctionalAssemblyExpression AssemblyCase*
        ( 'default' ':' AssemblyBlock )?
    AssemblyCase = 'case' FunctionalAssemblyExpression ':' AssemblyBlock
    AssemblyFunctionDefinition = 'function' Identifier '(' IdentifierList? ')'
        ( '->' '(' IdentifierList ')' )? AssemblyBlock
    AssemblyFor = 'for' ( AssemblyBlock | FunctionalAssemblyExpression)
        FunctionalAssemblyExpression ( AssemblyBlock | FunctionalAssemblyExpression) AssemblyBlock
    SubAssembly = 'assembly' Identifier AssemblyBlock
    LinkerSymbol = 'linkerSymbol' '(' StringLiteral ')'
    NumberLiteral = HexNumber | DecimalNumber
    HexLiteral = 'hex' ('"' ([0-9a-fA-F]{2})* '"' | '\'' ([0-9a-fA-F]{2})* '\'')
    StringLiteral = '"' ([^"\r\n\\] | '\\' .)* '"'
    HexNumber = '0x' [0-9a-fA-F]+
    DecimalNumber = [0-9]+


Desugaring
----------

An AST transformation removes for, switch and function constructs. The result
is still parseable by the same parser, but it will not use certain constructs.
If jumpdests are added that are only jumped to and not continued at, information
about the stack content is added, unless no local variables of outer scopes are
accessed or the stack height is the same as for the previous instruction.

Pseudocode::

    desugar item: AST -> AST =
    match item {
    AssemblyFunctionDefinition('function' name '(' arg1, ..., argn ')' '->' ( '(' ret1, ..., retm ')' body) ->
      <name>:
      {
        jump($<name>_start)
        let $retPC := 0 let argn := 0 ... let arg1 := 0
        $<name>_start:
        let ret1 := 0 ... let retm := 0
        { desugar(body) }
        swap and pop items so that only ret1, ... retm, $retPC are left on the stack
        jump
        0 (1 + n times) to compensate removal of arg1, ..., argn and $retPC
      }
    AssemblyFor('for' { init } condition post body) ->
      {
        init // cannot be its own block because we want variable scope to extend into the body
        // find I such that there are no labels $forI_*
        $forI_begin:
        jumpi($forI_end, iszero(condition))
        { body }
        $forI_continue:
        { post }
        jump($forI_begin)
        $forI_end:
      }
    'break' ->
      {
        // find nearest enclosing scope with label $forI_end
        pop all local variables that are defined at the current point
        but not at $forI_end
        jump($forI_end)
        0 (as many as variables were removed above)
      }
    'continue' ->
      {
        // find nearest enclosing scope with label $forI_continue
        pop all local variables that are defined at the current point
        but not at $forI_continue
        jump($forI_continue)
        0 (as many as variables were removed above)
      }
    AssemblySwitch(switch condition cases ( default: defaultBlock )? ) ->
      {
        // find I such that there is no $switchI* label or variable
        let $switchI_value := condition
        for each of cases match {
          case val: -> jumpi($switchI_caseJ, eq($switchI_value, val))
        }
        if default block present: ->
          { defaultBlock jump($switchI_end) }
        for each of cases match {
          case val: { body } -> $switchI_caseJ: { body jump($switchI_end) }
        }
        $switchI_end:
      }
    FunctionalAssemblyExpression( identifier(arg1, arg2, ..., argn) ) ->
      {
        if identifier is function <name> with n args and m ret values ->
          {
            // find I such that $funcallI_* does not exist
            $funcallI_return argn  ... arg2 arg1 jump(<name>)
            pop (n + 1 times)
            if the current context is `let (id1, ..., idm) := f(...)` ->
              let id1 := 0 ... let idm := 0
              $funcallI_return:
            else ->
              0 (m times)
              $funcallI_return:
              turn the functional expression that leads to the function call
              into a statement stream
          }
        else -> desugar(children of node)
      }
    default node ->
      desugar(children of node)
    }

Opcode Stream Generation
------------------------

During opcode stream generation, we keep track of the current stack height
in a counter,
so that accessing stack variables by name is possible. The stack height is modified with every opcode
that modifies the stack and with every label that is annotated with a stack
adjustment. Every time a new
local variable is introduced, it is registered together with the current
stack height. If a variable is accessed (either for copying its value or for
assignment), the appropriate DUP or SWAP instruction is selected depending
on the difference bitween the current stack height and the
stack height at the point the variable was introduced.

Pseudocode::

    codegen item: AST -> opcode_stream =
    match item {
    AssemblyBlock({ items }) ->
      join(codegen(item) for item in items)
      if last generated opcode has continuing control flow:
        POP for all local variables registered at the block (including variables
        introduced by labels)
        warn if the stack height at this point is not the same as at the start of the block
    Identifier(id) ->
      lookup id in the syntactic stack of blocks
      match type of id
        Local Variable ->
          DUPi where i = 1 + stack_height - stack_height_of_identifier(id)
        Label ->
          // reference to be resolved during bytecode generation
          PUSH<bytecode position of label>
        SubAssembly ->
          PUSH<bytecode position of subassembly data>
    FunctionalAssemblyExpression(id ( arguments ) ) ->
      join(codegen(arg) for arg in arguments.reversed())
      id (which has to be an opcode, might be a function name later)
    AssemblyLocalDefinition(let (id1, ..., idn) := expr) ->
      register identifiers id1, ..., idn as locals in current block at current stack height
      codegen(expr) - assert that expr returns n items to the stack
    FunctionalAssemblyAssignment((id1, ..., idn) := expr) ->
      lookup id1, ..., idn in the syntactic stack of blocks, assert that they are variables
      codegen(expr)
      for j = n, ..., i:
      SWAPi where i = 1 + stack_height - stack_height_of_identifier(idj)
      POP
    AssemblyAssignment(=: id) ->
      look up id in the syntactic stack of blocks, assert that it is a variable
      SWAPi where i = 1 + stack_height - stack_height_of_identifier(id)
      POP
    LabelDefinition(name:) ->
      JUMPDEST
    NumberLiteral(num) ->
      PUSH<num interpreted as decimal and right-aligned>
    HexLiteral(lit) ->
      PUSH32<lit interpreted as hex and left-aligned>
    StringLiteral(lit) ->
      PUSH32<lit utf-8 encoded and left-aligned>
    SubAssembly(assembly <name> block) ->
      append codegen(block) at the end of the code
    dataSize(<name>) ->
      assert that <name> is a subassembly ->
      PUSH32<size of code generated from subassembly <name>>
    linkerSymbol(<lit>) ->
      PUSH32<zeros> and append position to linker table
    }
