###
Yul
###

.. _yul:

.. index:: ! assembly, ! asm, ! evmasm, ! yul, julia, iulia

Yul (previously also called JULIA or IULIA) is an intermediate language that can be
compiled to bytecode for different backends.

Support for EVM 1.0, EVM 1.5 and eWASM is planned, and it is designed to be a usable common denominator of all three
platforms. It can already be used for "inline assembly" inside Solidity and future versions of the Solidity compiler
will use Yul as an intermediate language. Yul is a good target for high-level optimisation stages that can benefit all target platforms equally.

With the "inline assembly" flavour, Yul can be used as a language setting
for the :ref:`standard-json interface <compiler-api>`:

::

    {
        "language": "Yul",
        "sources": { "input.yul": { "content": "{ sstore(0, 1) }" } },
        "settings": {
            "outputSelection": { "*": { "*": ["*"], "": [ "*" ] } },
            "optimizer": { "enabled": true, "details": { "yul": true } }
        }
    }

And on the command line interface with the ``--strict-assembly`` parameter.

.. warning::

    Yul is in active development and bytecode generation is fully implemented only for untyped Yul (everything is ``u256``)
    and with EVM 1.0 as target, :ref:`EVM opcodes <opcodes>` are used as built-in functions.

The core components of Yul are functions, blocks, variables, literals,
for-loops, if-statements, switch-statements, expressions and assignments to variables.

Yul is typed, both variables and literals must specify the type with postfix
notation. The supported types are ``bool``, ``u8``, ``s8``, ``u32``, ``s32``,
``u64``, ``s64``, ``u128``, ``s128``, ``u256`` and ``s256``.

Yul in itself does not even provide operators. If the EVM is targeted,
opcodes will be available as built-in functions, but they can be reimplemented
if the backend changes. For a list of mandatory built-in functions, see the section below.

The following example program assumes that the EVM opcodes ``mul``, ``div``
and ``mod`` are available either natively or as functions and computes exponentiation.
As per the warning above, the following code is untyped and can be compiled using ``solc --strict-assembly``.

.. code::

    {
        function power(base, exponent) -> result
        {
            switch exponent
            case 0 { result := 1 }
            case 1 { result := base }
            default
            {
                result := power(mul(base, base), div(exponent, 2))
                switch mod(exponent, 2)
                    case 1 { result := mul(base, result) }
            }
        }
    }

It is also possible to implement the same function using a for-loop
instead of with recursion. Here, we need the EVM opcodes ``lt`` (less-than)
and ``add`` to be available.

.. code::

    {
        function power(base, exponent) -> result
        {
            result := 1
            for { let i := 0 } lt(i, exponent) { i := add(i, 1) }
            {
                result := mul(result, base)
            }
        }
    }

Specification of Yul
====================

This chapter describes Yul code. It is usually placed inside a Yul object, which is described in the following chapter.

Grammar::

    Block = '{' Statement* '}'
    Statement =
        Block |
        FunctionDefinition |
        VariableDeclaration |
        Assignment |
        If |
        Expression |
        Switch |
        ForLoop |
        BreakContinue |
        Leave
    FunctionDefinition =
        'function' Identifier '(' TypedIdentifierList? ')'
        ( '->' TypedIdentifierList )? Block
    VariableDeclaration =
        'let' TypedIdentifierList ( ':=' Expression )?
    Assignment =
        IdentifierList ':=' Expression
    Expression =
        FunctionCall | Identifier | Literal
    If =
        'if' Expression Block
    Switch =
        'switch' Expression ( Case+ Default? | Default )
    Case =
        'case' Literal Block
    Default =
        'default' Block
    ForLoop =
        'for' Block Expression Block Block
    BreakContinue =
        'break' | 'continue'
    Leave = 'leave'
    FunctionCall =
        Identifier '(' ( Expression ( ',' Expression )* )? ')'
    Identifier = [a-zA-Z_$] [a-zA-Z_$0-9.]*
    IdentifierList = Identifier ( ',' Identifier)*
    TypeName = Identifier | BuiltinTypeName
    BuiltinTypeName = 'bool' | [us] ( '8' | '32' | '64' | '128' | '256' )
    TypedIdentifierList = Identifier ':' TypeName ( ',' Identifier ':' TypeName )*
    Literal =
        (NumberLiteral | StringLiteral | HexLiteral | TrueLiteral | FalseLiteral) ':' TypeName
    NumberLiteral = HexNumber | DecimalNumber
    HexLiteral = 'hex' ('"' ([0-9a-fA-F]{2})* '"' | '\'' ([0-9a-fA-F]{2})* '\'')
    StringLiteral = '"' ([^"\r\n\\] | '\\' .)* '"'
    TrueLiteral = 'true'
    FalseLiteral = 'false'
    HexNumber = '0x' [0-9a-fA-F]+
    DecimalNumber = [0-9]+

Restrictions on the Grammar
---------------------------

Switches must have at least one case (including the default case).
If all possible values of the expression are covered, a default case should
not be allowed (i.e. a switch with a ``bool`` expression that has both a
true and a false case should not allow a default case). All case values need to
have the same type.

Every expression evaluates to zero or more values. Identifiers and Literals
evaluate to exactly
one value and function calls evaluate to a number of values equal to the
number of return values of the function called.

In variable declarations and assignments, the right-hand-side expression
(if present) has to evaluate to a number of values equal to the number of
variables on the left-hand-side.
This is the only situation where an expression evaluating
to more than one value is allowed.

Expressions that are also statements (i.e. at the block level) have to
evaluate to zero values.

In all other situations, expressions have to evaluate to exactly one value.

The ``continue`` and ``break`` statements can only be used inside loop bodies
and have to be in the same function as the loop (or both have to be at the
top level).
The ``leave`` statement can only be used inside a function.
The condition part of the for-loop has to evaluate to exactly one value.
Functions cannot be defined inside for loop init blocks.

Literals cannot be larger than the their type. The largest type defined is 256-bit wide.

Scoping Rules
-------------

Scopes in Yul are tied to Blocks (exceptions are functions and the for loop
as explained below) and all declarations
(``FunctionDefinition``, ``VariableDeclaration``)
introduce new identifiers into these scopes.

Identifiers are visible in
the block they are defined in (including all sub-nodes and sub-blocks).

As an exception, identifiers defined directly in the "init" part of the for-loop
(the first block) are visible in all other parts of the for-loop
(but not outside of the loop).
Identifiers declared in the other parts of the for loop respect the regular
syntactical scoping rules.

This means a for-loop of the form ``for { I... } C { P... } { B... }`` is equivalent
to ``{ I... for {} C { P... } { B... } }``.


The parameters and return parameters of functions are visible in the
function body and their names cannot overlap.

Variables can only be referenced after their declaration. In particular,
variables cannot be referenced in the right hand side of their own variable
declaration.
Functions can be referenced already before their declaration (if they are visible).

Shadowing is disallowed, i.e. you cannot declare an identifier at a point
where another identifier with the same name is also visible, even if it is
not accessible.

Inside functions, it is not possible to access a variable that was declared
outside of that function.

Formal Specification
--------------------

We formally specify Yul by providing an evaluation function E overloaded
on the various nodes of the AST. Any functions can have side effects, so
E takes two state objects and the AST node and returns two new
state objects and a variable number of other values.
The two state objects are the global state object
(which in the context of the EVM is the memory, storage and state of the
blockchain) and the local state object (the state of local variables, i.e. a
segment of the stack in the EVM).
If the AST node is a statement, E returns the two state objects and a "mode",
which is used for the ``break``, ``continue`` and ``leave`` statements.
If the AST node is an expression, E returns the two state objects and
as many values as the expression evaluates to.


The exact nature of the global state is unspecified for this high level
description. The local state ``L`` is a mapping of identifiers ``i`` to values ``v``,
denoted as ``L[i] = v``.

For an identifier ``v``, let ``$v`` be the name of the identifier.

We will use a destructuring notation for the AST nodes.

.. code::

    E(G, L, <{St1, ..., Stn}>: Block) =
        let G1, L1, mode = E(G, L, St1, ..., Stn)
        let L2 be a restriction of L1 to the identifiers of L
        G1, L2, mode
    E(G, L, St1, ..., Stn: Statement) =
        if n is zero:
            G, L, regular
        else:
            let G1, L1, mode = E(G, L, St1)
            if mode is regular then
                E(G1, L1, St2, ..., Stn)
            otherwise
                G1, L1, mode
    E(G, L, FunctionDefinition) =
        G, L, regular
    E(G, L, <let var_1, ..., var_n := rhs>: VariableDeclaration) =
        E(G, L, <var_1, ..., var_n := rhs>: Assignment)
    E(G, L, <let var_1, ..., var_n>: VariableDeclaration) =
        let L1 be a copy of L where L1[$var_i] = 0 for i = 1, ..., n
        G, L1, regular
    E(G, L, <var_1, ..., var_n := rhs>: Assignment) =
        let G1, L1, v1, ..., vn = E(G, L, rhs)
        let L2 be a copy of L1 where L2[$var_i] = vi for i = 1, ..., n
        G, L2, regular
    E(G, L, <for { i1, ..., in } condition post body>: ForLoop) =
        if n >= 1:
            let G1, L, mode = E(G, L, i1, ..., in)
            // mode has to be regular or leave due to the syntactic restrictions
            if mode is leave then
                G1, L1 restricted to variables of L, leave
            otherwise
                let G2, L2, mode = E(G1, L1, for {} condition post body)
                G2, L2 restricted to variables of L, mode
        else:
            let G1, L1, v = E(G, L, condition)
            if v is false:
                G1, L1, regular
            else:
                let G2, L2, mode = E(G1, L, body)
                if mode is break:
                    G2, L2, regular
                otherwise if mode is leave:
                    G2, L2, leave
                else:
                    G3, L3, mode = E(G2, L2, post)
                    if mode is leave:
                        G2, L3, leave
                    otherwise
                        E(G3, L3, for {} condition post body)
    E(G, L, break: BreakContinue) =
        G, L, break
    E(G, L, continue: BreakContinue) =
        G, L, continue
    E(G, L, leave: Leave) =
        G, L, leave
    E(G, L, <if condition body>: If) =
        let G0, L0, v = E(G, L, condition)
        if v is true:
            E(G0, L0, body)
        else:
            G0, L0, regular
    E(G, L, <switch condition case l1:t1 st1 ... case ln:tn stn>: Switch) =
        E(G, L, switch condition case l1:t1 st1 ... case ln:tn stn default {})
    E(G, L, <switch condition case l1:t1 st1 ... case ln:tn stn default st'>: Switch) =
        let G0, L0, v = E(G, L, condition)
        // i = 1 .. n
        // Evaluate literals, context doesn't matter
        let _, _, v1 = E(G0, L0, l1)
        ...
        let _, _, vn = E(G0, L0, ln)
        if there exists smallest i such that vi = v:
            E(G0, L0, sti)
        else:
            E(G0, L0, st')

    E(G, L, <name>: Identifier) =
        G, L, L[$name]
    E(G, L, <fname(arg1, ..., argn)>: FunctionCall) =
        G1, L1, vn = E(G, L, argn)
        ...
        G(n-1), L(n-1), v2 = E(G(n-2), L(n-2), arg2)
        Gn, Ln, v1 = E(G(n-1), L(n-1), arg1)
        Let <function fname (param1, ..., paramn) -> ret1, ..., retm block>
        be the function of name $fname visible at the point of the call.
        Let L' be a new local state such that
        L'[$parami] = vi and L'[$reti] = 0 for all i.
        Let G'', L'', mode = E(Gn, L', block)
        G'', Ln, L''[$ret1], ..., L''[$retm]
    E(G, L, l: HexLiteral) = G, L, hexString(l),
        where hexString decodes l from hex and left-aligns it into 32 bytes
    E(G, L, l: StringLiteral) = G, L, utf8EncodeLeftAligned(l),
        where utf8EncodeLeftAligned performs a utf8 encoding of l
        and aligns it left into 32 bytes
    E(G, L, n: HexNumber) = G, L, hex(n)
        where hex is the hexadecimal decoding function
    E(G, L, n: DecimalNumber) = G, L, dec(n),
        where dec is the decimal decoding function

Type Conversion Functions
-------------------------

Yul has no support for implicit type conversion and therefore functions exist to provide explicit conversion.
When converting a larger type to a shorter type a runtime exception can occur in case of an overflow.

Truncating conversions are supported between the following types:
 - ``bool``
 - ``u32``
 - ``u64``
 - ``u256``
 - ``s256``

For each of these a type conversion function exists having the prototype in the form of ``<input_type>to<output_type>(x:<input_type>) -> y:<output_type>``,
such as ``u32tobool(x:u32) -> y:bool``, ``u256tou32(x:u256) -> y:u32`` or ``s256tou256(x:s256) -> y:u256``.

.. note::

    ``u32tobool(x:u32) -> y:bool`` can be implemented as ``y := not(iszerou256(x))`` and
    ``booltou32(x:bool) -> y:u32`` can be implemented as ``switch x case true:bool { y := 1:u32 } case false:bool { y := 0:u32 }``

Low-level Functions
-------------------

The following functions must be available:

+---------------------------------------------------------------------------------------------------------------+
| *Logic*                                                                                                       |
+---------------------------------------------+-----------------------------------------------------------------+
| not(x:bool) ‑> z:bool                       | logical not                                                     |
+---------------------------------------------+-----------------------------------------------------------------+
| and(x:bool, y:bool) ‑> z:bool               | logical and                                                     |
+---------------------------------------------+-----------------------------------------------------------------+
| or(x:bool, y:bool) ‑> z:bool                | logical or                                                      |
+---------------------------------------------+-----------------------------------------------------------------+
| xor(x:bool, y:bool) ‑> z:bool               | xor                                                             |
+---------------------------------------------+-----------------------------------------------------------------+
| *Arithmetic*                                                                                                  |
+---------------------------------------------+-----------------------------------------------------------------+
| addu256(x:u256, y:u256) ‑> z:u256           | x + y                                                           |
+---------------------------------------------+-----------------------------------------------------------------+
| subu256(x:u256, y:u256) ‑> z:u256           | x - y                                                           |
+---------------------------------------------+-----------------------------------------------------------------+
| mulu256(x:u256, y:u256) ‑> z:u256           | x * y                                                           |
+---------------------------------------------+-----------------------------------------------------------------+
| divu256(x:u256, y:u256) ‑> z:u256           | x / y                                                           |
+---------------------------------------------+-----------------------------------------------------------------+
| divs256(x:s256, y:s256) ‑> z:s256           | x / y, for signed numbers in two's complement                   |
+---------------------------------------------+-----------------------------------------------------------------+
| modu256(x:u256, y:u256) ‑> z:u256           | x % y                                                           |
+---------------------------------------------+-----------------------------------------------------------------+
| mods256(x:s256, y:s256) ‑> z:s256           | x % y, for signed numbers in two's complement                   |
+---------------------------------------------+-----------------------------------------------------------------+
| signextendu256(i:u256, x:u256) ‑> z:u256    | sign extend from (i*8+7)th bit counting from least significant  |
+---------------------------------------------+-----------------------------------------------------------------+
| expu256(x:u256, y:u256) ‑> z:u256           | x to the power of y                                             |
+---------------------------------------------+-----------------------------------------------------------------+
| addmodu256(x:u256, y:u256, m:u256) ‑> z:u256| (x + y) % m with arbitrary precision arithmetic                 |
+---------------------------------------------+-----------------------------------------------------------------+
| mulmodu256(x:u256, y:u256, m:u256) ‑> z:u256| (x * y) % m with arbitrary precision arithmetic                 |
+---------------------------------------------+-----------------------------------------------------------------+
| ltu256(x:u256, y:u256) ‑> z:bool            | true if x < y, false otherwise                                  |
+---------------------------------------------+-----------------------------------------------------------------+
| gtu256(x:u256, y:u256) ‑> z:bool            | true if x > y, false otherwise                                  |
+---------------------------------------------+-----------------------------------------------------------------+
| lts256(x:s256, y:s256) ‑> z:bool            | true if x < y, false otherwise                                  |
|                                             | (for signed numbers in two's complement)                        |
+---------------------------------------------+-----------------------------------------------------------------+
| gts256(x:s256, y:s256) ‑> z:bool            | true if x > y, false otherwise                                  |
|                                             | (for signed numbers in two's complement)                        |
+---------------------------------------------+-----------------------------------------------------------------+
| equ256(x:u256, y:u256) ‑> z:bool            | true if x == y, false otherwise                                 |
+---------------------------------------------+-----------------------------------------------------------------+
| iszerou256(x:u256) ‑> z:bool                | true if x == 0, false otherwise                                 |
+---------------------------------------------+-----------------------------------------------------------------+
| notu256(x:u256) ‑> z:u256                   | ~x, every bit of x is negated                                   |
+---------------------------------------------+-----------------------------------------------------------------+
| andu256(x:u256, y:u256) ‑> z:u256           | bitwise and of x and y                                          |
+---------------------------------------------+-----------------------------------------------------------------+
| oru256(x:u256, y:u256) ‑> z:u256            | bitwise or of x and y                                           |
+---------------------------------------------+-----------------------------------------------------------------+
| xoru256(x:u256, y:u256) ‑> z:u256           | bitwise xor of x and y                                          |
+---------------------------------------------+-----------------------------------------------------------------+
| shlu256(x:u256, y:u256) ‑> z:u256           | logical left shift of x by y                                    |
+---------------------------------------------+-----------------------------------------------------------------+
| shru256(x:u256, y:u256) ‑> z:u256           | logical right shift of x by y                                   |
+---------------------------------------------+-----------------------------------------------------------------+
| sars256(x:s256, y:u256) ‑> z:u256           | arithmetic right shift of x by y                                |
+---------------------------------------------+-----------------------------------------------------------------+
| byte(n:u256, x:u256) ‑> v:u256              | nth byte of x, where the most significant byte is the 0th byte  |
|                                             | Cannot this be just replaced by and256(shr256(n, x), 0xff) and  |
|                                             | let it be optimised out by the EVM backend?                     |
+---------------------------------------------+-----------------------------------------------------------------+
| *Memory and storage*                                                                                          |
+---------------------------------------------+-----------------------------------------------------------------+
| mload(p:u256) ‑> v:u256                     | mem[p..(p+32))                                                  |
+---------------------------------------------+-----------------------------------------------------------------+
| mstore(p:u256, v:u256)                      | mem[p..(p+32)) := v                                             |
+---------------------------------------------+-----------------------------------------------------------------+
| mstore8(p:u256, v:u256)                     | mem[p] := v & 0xff    - only modifies a single byte             |
+---------------------------------------------+-----------------------------------------------------------------+
| sload(p:u256) ‑> v:u256                     | storage[p]                                                      |
+---------------------------------------------+-----------------------------------------------------------------+
| sstore(p:u256, v:u256)                      | storage[p] := v                                                 |
+---------------------------------------------+-----------------------------------------------------------------+
| msize() ‑> size:u256                        | size of memory, i.e. largest accessed memory index, albeit due  |
|                                             | due to the memory extension function, which extends by words,   |
|                                             | this will always be a multiple of 32 bytes                      |
+---------------------------------------------+-----------------------------------------------------------------+
| *Execution control*                                                                                           |
+---------------------------------------------+-----------------------------------------------------------------+
| create(v:u256, p:u256, n:u256)              | create new contract with code mem[p..(p+n)) and send v wei      |
|                                             | and return the new address                                      |
+---------------------------------------------+-----------------------------------------------------------------+
| create2(v:u256, p:u256, n:u256, s:u256)     | create new contract with code mem[p...(p+n)) at address         |
|                                             | keccak256(0xff . this . s . keccak256(mem[p...(p+n)))           |
|                                             | and send v wei and return the new address, where ``0xff`` is a  |
|                                             | 8 byte value, ``this`` is the current contract's address        |
|                                             | as a 20 byte value and ``s`` is a big-endian 256-bit value      |
+---------------------------------------------+-----------------------------------------------------------------+
| call(g:u256, a:u256, v:u256, in:u256,       | call contract at address a with input mem[in..(in+insize))      |
| insize:u256, out:u256,                      | providing g gas and v wei and output area                       |
| outsize:u256)                               | mem[out..(out+outsize)) returning 0 on error (eg. out of gas)   |
| ‑> r:u256                                   | and 1 on success                                                |
+---------------------------------------------+-----------------------------------------------------------------+
| callcode(g:u256, a:u256, v:u256, in:u256,   | identical to ``call`` but only use the code from a              |
| insize:u256, out:u256,                      | and stay in the context of the                                  |
| outsize:u256) ‑> r:u256                     | current contract otherwise                                      |
+---------------------------------------------+-----------------------------------------------------------------+
| delegatecall(g:u256, a:u256, in:u256,       | identical to ``callcode``,                                      |
| insize:u256, out:u256,                      | but also keep ``caller``                                        |
| outsize:u256) ‑> r:u256                     | and ``callvalue``                                               |
+---------------------------------------------+-----------------------------------------------------------------+
| abort()                                     | abort (equals to invalid instruction on EVM)                    |
+---------------------------------------------+-----------------------------------------------------------------+
| return(p:u256, s:u256)                      | end execution, return data mem[p..(p+s))                        |
+---------------------------------------------+-----------------------------------------------------------------+
| revert(p:u256, s:u256)                      | end execution, revert state changes, return data mem[p..(p+s))  |
+---------------------------------------------+-----------------------------------------------------------------+
| selfdestruct(a:u256)                        | end execution, destroy current contract and send funds to a     |
+---------------------------------------------+-----------------------------------------------------------------+
| log0(p:u256, s:u256)                        | log without topics and data mem[p..(p+s))                       |
+---------------------------------------------+-----------------------------------------------------------------+
| log1(p:u256, s:u256, t1:u256)               | log with topic t1 and data mem[p..(p+s))                        |
+---------------------------------------------+-----------------------------------------------------------------+
| log2(p:u256, s:u256, t1:u256, t2:u256)      | log with topics t1, t2 and data mem[p..(p+s))                   |
+---------------------------------------------+-----------------------------------------------------------------+
| log3(p:u256, s:u256, t1:u256, t2:u256,      | log with topics t, t2, t3 and data mem[p..(p+s))                |
| t3:u256)                                    |                                                                 |
+---------------------------------------------+-----------------------------------------------------------------+
| log4(p:u256, s:u256, t1:u256, t2:u256,      | log with topics t1, t2, t3, t4 and data mem[p..(p+s))           |
| t3:u256, t4:u256)                           |                                                                 |
+---------------------------------------------+-----------------------------------------------------------------+
| *State queries*                                                                                               |
+---------------------------------------------+-----------------------------------------------------------------+
| blockcoinbase() ‑> address:u256             | current mining beneficiary                                      |
+---------------------------------------------+-----------------------------------------------------------------+
| blockdifficulty() ‑> difficulty:u256        | difficulty of the current block                                 |
+---------------------------------------------+-----------------------------------------------------------------+
| blockgaslimit() ‑> limit:u256               | block gas limit of the current block                            |
+---------------------------------------------+-----------------------------------------------------------------+
| blockhash(b:u256) ‑> hash:u256              | hash of block nr b - only for last 256 blocks excluding current |
+---------------------------------------------+-----------------------------------------------------------------+
| blocknumber() ‑> block:u256                 | current block number                                            |
+---------------------------------------------+-----------------------------------------------------------------+
| blocktimestamp() ‑> timestamp:u256          | timestamp of the current block in seconds since the epoch       |
+---------------------------------------------+-----------------------------------------------------------------+
| txorigin() ‑> address:u256                  | transaction sender                                              |
+---------------------------------------------+-----------------------------------------------------------------+
| txgasprice() ‑> price:u256                  | gas price of the transaction                                    |
+---------------------------------------------+-----------------------------------------------------------------+
| gasleft() ‑> gas:u256                       | gas still available to execution                                |
+---------------------------------------------+-----------------------------------------------------------------+
| balance(a:u256) ‑> v:u256                   | wei balance at address a                                        |
+---------------------------------------------+-----------------------------------------------------------------+
| this() ‑> address:u256                      | address of the current contract / execution context             |
+---------------------------------------------+-----------------------------------------------------------------+
| caller() ‑> address:u256                    | call sender (excluding delegatecall)                            |
+---------------------------------------------+-----------------------------------------------------------------+
| callvalue() ‑> v:u256                       | wei sent together with the current call                         |
+---------------------------------------------+-----------------------------------------------------------------+
| calldataload(p:u256) ‑> v:u256              | call data starting from position p (32 bytes)                   |
+---------------------------------------------+-----------------------------------------------------------------+
| calldatasize() ‑> v:u256                    | size of call data in bytes                                      |
+---------------------------------------------+-----------------------------------------------------------------+
| calldatacopy(t:u256, f:u256, s:u256)        | copy s bytes from calldata at position f to mem at position t   |
+---------------------------------------------+-----------------------------------------------------------------+
| codesize() ‑> size:u256                     | size of the code of the current contract / execution context    |
+---------------------------------------------+-----------------------------------------------------------------+
| codecopy(t:u256, f:u256, s:u256)            | copy s bytes from code at position f to mem at position t       |
+---------------------------------------------+-----------------------------------------------------------------+
| extcodesize(a:u256) ‑> size:u256            | size of the code at address a                                   |
+---------------------------------------------+-----------------------------------------------------------------+
| extcodecopy(a:u256, t:u256, f:u256, s:u256) | like codecopy(t, f, s) but take code at address a               |
+---------------------------------------------+-----------------------------------------------------------------+
| extcodehash(a:u256)                         | code hash of address a                                          |
+---------------------------------------------+-----------------------------------------------------------------+
| *Others*                                                                                                      |
+---------------------------------------------+-----------------------------------------------------------------+
| discard(unused:bool)                        | discard value                                                   |
+---------------------------------------------+-----------------------------------------------------------------+
| discardu256(unused:u256)                    | discard value                                                   |
+---------------------------------------------+-----------------------------------------------------------------+
| splitu256tou64(x:u256) ‑> (x1:u64, x2:u64,  | split u256 to four u64's                                        |
| x3:u64, x4:u64)                             |                                                                 |
+---------------------------------------------+-----------------------------------------------------------------+
| combineu64tou256(x1:u64, x2:u64, x3:u64,    | combine four u64's into a single u256                           |
| x4:u64) ‑> (x:u256)                         |                                                                 |
+---------------------------------------------+-----------------------------------------------------------------+
| keccak256(p:u256, s:u256) ‑> v:u256         | keccak(mem[p...(p+s)))                                          |
+---------------------------------------------+-----------------------------------------------------------------+
| *Object access*                             |                                                                 |
+---------------------------------------------+-----------------------------------------------------------------+
| datasize(name:string) ‑> size:u256          | size of the data object in bytes, name has to be string literal |
+---------------------------------------------+-----------------------------------------------------------------+
| dataoffset(name:string) ‑> offset:u256      | offset of the data object inside the data area in bytes,        |
|                                             | name has to be string literal                                   |
+---------------------------------------------+-----------------------------------------------------------------+
| datacopy(dst:u256, src:u256, len:u256)      | copy len bytes from the data area starting at offset src bytes  |
|                                             | to memory at position dst                                       |
+---------------------------------------------+-----------------------------------------------------------------+

Backends
--------

Backends or targets are the translators from Yul to a specific bytecode. Each of the backends can expose functions
prefixed with the name of the backend. We reserve ``evm_`` and ``ewasm_`` prefixes for the two proposed backends.

Backend: EVM
------------

The EVM target will have all the underlying EVM opcodes exposed with the `evm_` prefix.

Backend: "EVM 1.5"
------------------

TBD

Backend: eWASM
--------------

TBD

Specification of Yul Object
===========================

Yul objects are used to group named code and data sections.
The functions ``datasize``, ``dataoffset`` and ``datacopy``
can be used to access these sections from within code.
Hex strings can be used to specify data in hex encoding,
regular strings in native encoding. For code,
``datacopy`` will access its assembled binary representation.

Grammar::

    Object = 'object' StringLiteral '{' Code ( Object | Data )* '}'
    Code = 'code' Block
    Data = 'data' StringLiteral ( HexLiteral | StringLiteral )
    HexLiteral = 'hex' ('"' ([0-9a-fA-F]{2})* '"' | '\'' ([0-9a-fA-F]{2})* '\'')
    StringLiteral = '"' ([^"\r\n\\] | '\\' .)* '"'

Above, ``Block`` refers to ``Block`` in the Yul code grammar explained in the previous chapter.

An example Yul Object is shown below:

.. code::

    // Code consists of a single object. A single "code" node is the code of the object.
    // Every (other) named object or data section is serialized and
    // made accessible to the special built-in functions datacopy / dataoffset / datasize
    // Access to nested objects can be performed by joining the names using ``.``.
    // The current object and sub-objects and data items inside the current object
    // are in scope without nested access.
    object "Contract1" {
        code {
            function allocate(size) -> ptr {
                ptr := mload(0x40)
                if iszero(ptr) { ptr := 0x60 }
                mstore(0x40, add(ptr, size))
            }

            // first create "runtime.Contract2"
            let size := datasize("runtime.Contract2")
            let offset := allocate(size)
            // This will turn into a memory->memory copy for eWASM and
            // a codecopy for EVM
            datacopy(offset, dataoffset("runtime.Contract2"), size)
            // constructor parameter is a single number 0x1234
            mstore(add(offset, size), 0x1234)
            pop(create(offset, add(size, 32), 0))

            // now return the runtime object (this is
            // constructor code)
            size := datasize("runtime")
            offset := allocate(size)
            // This will turn into a memory->memory copy for eWASM and
            // a codecopy for EVM
            datacopy(offset, dataoffset("runtime"), size)
            return(offset, size)
        }

        data "Table2" hex"4123"

        object "runtime" {
            code {
                function allocate(size) -> ptr {
                    ptr := mload(0x40)
                    if iszero(ptr) { ptr := 0x60 }
                    mstore(0x40, add(ptr, size))
                }

                // runtime code

                let size := datasize("Contract2")
                let offset := allocate(size)
                // This will turn into a memory->memory copy for eWASM and
                // a codecopy for EVM
                datacopy(offset, dataoffset("Contract2"), size)
                // constructor parameter is a single number 0x1234
                mstore(add(offset, size), 0x1234)
                pop(create(offset, add(size, 32), 0))
            }

            // Embedded object. Use case is that the outside is a factory contract,
            // and Contract2 is the code to be created by the factory
            object "Contract2" {
                code {
                    // code here ...
                }

                object "runtime" {
                    code {
                        // code here ...
                    }
                }

                data "Table1" hex"4123"
            }
        }
    }
