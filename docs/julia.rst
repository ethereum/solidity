#################################################
Joyfully Universal Language for (Inline) Assembly
#################################################

.. _julia:

.. index:: ! assembly, ! asm, ! evmasm, ! julia

JULIA is an intermediate language that can compile to various different backends
(EVM 1.0, EVM 1.5 and eWASM are planned).
Because of that, it is designed to be as featureless as possible.
It can already be used for "inline assembly" inside Solidity and
future versions of the Solidity compiler will even use JULIA as intermediate
language. It should also be easy to build high-level optimizer stages for JULIA.

The core components of JULIA are functions, blocks, variables, literals,
for-loops, switch-statements, expressions and assignments to variables.

JULIA is typed, both variables and literals must specify the type with postfix
notation. The supported types are ``bool``, ``u8``, ``s8``, ``u32``, ``s32``,
``u64``, ``s64``, ``u128``, ``s128``, ``u256`` and ``s256``.

JULIA in itself does not even provide operators. If the EVM is targeted,
opcodes will be available as built-in functions, but they can be reimplemented
if the backend changes. For a list of mandatory built-in functions, see the section below.

The following example program assumes that the EVM opcodes ``mul``, ``div``
and ``mod`` are available either natively or as functions and computes exponentiation.

.. code::

    {
        function power(base:u256, exponent:u256) -> (result:u256)
        {
            switch exponent
            case 0:u256: { result := 1:u256 }
            case 1:u256: { result := base }
            default:
            {
                result := power(mul(base, base), div(exponent, 2:u256))
                switch mod(exponent, 2:u256)
                    case 1:u256: { result := mul(base, result) }
            }
        }
    }

It is also possible to implement the same function using a for-loop
instead of recursion. Here, we need the EVM opcodes ``lt`` (less-than)
and ``add`` to be available.

.. code::

    {
        function power(base:u256, exponent:u256) -> (result:u256)
        {
            result := 1:u256
            for { let i := 0:u256 } lt(i, exponent) { i := add(i, 1:u256) }
            {
                result := mul(result, base)
            }
        }
    }

Specification of JULIA
======================

Grammar::

    Block = '{' Statement* '}'
    Statement =
        Block |
        FunctionDefinition |
        VariableDeclaration |
        Assignment |
        Expression |
        Switch |
        ForLoop |
        BreakContinue |
        SubAssembly
    FunctionDefinition =
        'function' Identifier '(' IdentifierList? ')'
        ( '->' '(' IdentifierList ')' )? Block
    VariableDeclaration =
        'let' IdentifierOrList ':=' Expression
    Assignment =
        IdentifierOrList ':=' Expression
    Expression =
        FunctionCall | Identifier | Literal
    Switch =
        'switch' Expression Case* ( 'default' ':' Block )?
    Case =
        'case' Expression ':' Block
    ForLoop =
        'for' Block Expression Block Block
    BreakContinue =
        'break' | 'continue'
    SubAssembly =
        'assembly' Identifier Block
    FunctionCall =
        Identifier '(' ( Expression ( ',' Expression )* )? ')'
    IdentifierOrList = Identifier | '(' IdentifierList ')'
    Identifier = [a-zA-Z_$] [a-zA-Z_0-9]*
    IdentifierList = Identifier ( ',' Identifier)*
    Literal =
        NumberLiteral | StringLiteral | HexLiteral
    NumberLiteral = HexNumber | DecimalNumber
    HexLiteral = 'hex' ('"' ([0-9a-fA-F]{2})* '"' | '\'' ([0-9a-fA-F]{2})* '\'')
    StringLiteral = '"' ([^"\r\n\\] | '\\' .)* '"'
    HexNumber = '0x' [0-9a-fA-F]+
    DecimalNumber = [0-9]+

Restrictions on the Grammar
---------------------------

Scopes in JULIA are tied to Blocks and all declarations
(``FunctionDefinition``, ``VariableDeclaration`` and ``SubAssembly``)
introduce new identifiers into these scopes. Shadowing is disallowed

Talk about identifiers across functions etc

Restriction for Expression: Statements have to return empty tuple
Function arguments have to be single item

Restriction for VariableDeclaration and Assignment: Number of elements left and right needs to be the same
continue and break only in for loop

Literals have to fit 32 bytes

Formal Specification
--------------------

We formally specify JULIA by providing an evaluation function E overloaded
on the various nodes of the AST. Any functions can have side effects, so
E takes a state objects and the actual argument and also returns new
state objects and new arguments. There is a global state object
(which in the context of the EVM is the memory, storage and state of the
blockchain) and a local state object (the state of local variables, i.e. a
segment of the stack in the EVM).

The the evaluation function E takes a global state, a local state and
a node of the AST and returns a new global state, a new local state
and a value (if the AST node is an expression).

We use sequence numbers as a shorthand for the order of evaluation
and how state is forwarded. For example, ``E2(x), E1(y)`` is a shorthand
for

For ``(S1, z) = E(S, y)`` let ``(S2, w) = E(S1, x)``. TODO

.. code::

    E(G, L, <{St1, ..., Stn}>: Block) =
        let L' be a copy of L that adds a new inner scope which contains
        all functions and variables declared in the block (but not its sub-blocks)
        variables are marked inactive for now
        TODO: more formal
        G1, L'1 = E(G, L', St1)
        G2, L'2 = E(G1, L'1, St2)
        ...
        Gn, L'n = E(G(n-1), L'(n-1), Stn)
        let L'' be a copy of L'n where the innermost scope is removed
        Gn, L''
    E(G, L, <function fname (param1, ..., paramn) -> (ret1, ..., retm) block>: FunctionDefinition) =
        G, L
    E(G, L, <let (var1, ..., varn) := value>: VariableDeclaration) =
        E(G, L, <(var1, ..., varn) := value>: Assignment)
    E(G, L, <(var1, ..., varn) := value>: Assignment) =
        let G', L', v1, ..., vn = E(G, L, value)
        let L'' be a copy of L' where L'[vi] = vi for i = 1, ..., n
        G, L''
    E(G, L, name: Identifier) =
        G, L, L[name]
    E(G, L, fname(arg1, ..., argn): FunctionCall) =
        G1, L1, vn = E(G, L, argn)
        ...
        G(n-1), L(n-1), v2 = E(G(n-2), L(n-2), arg2)
        Gn, Ln, v1 = E(G(n-1), L(n-1), arg1)
        Let <function fname (param1, ..., paramn) -> (ret1, ..., retm) block>
        be the function L[fname].
        Let L' be a copy of L that does not contain any variables in any scope,
        but which has a new innermost scope such that
        L'[parami] = vi and L'[reti] = 0
        Let G'', L'', rv1, ..., rvm = E(Gn, L', block)
        G'', Ln, rv1, ..., rvm
    E(G, L, l: HexLiteral) = G, L, hexString(l),
        where hexString decodes l from hex and left-aligns in into 32 bytes
    E(G, L, l: StringLiteral) = G, L, utf8EncodeLeftAligned(l),
        where utf8EncodeLeftAligned performs a utf8 encoding of l
        and aligns it left into 32 bytes
    E(G, L, n: HexNumber) = G, L, hex(n)
        where hex is the hexadecimal decoding function
    E(G, L, n: DecimalNumber) = G, L, dec(n),
        where dec is the decimal decoding function

Type Conversion Functions
-------------------------

JULIA has no support for implicit type conversion and therefore functions exists to provide explicit conversion.
When converting a larger type to a shorter type a runtime exception can occur in case of an overflow.

The following type conversion functions must be available:
- ``u32tobool(x:u32) -> (y:bool)``
- ``booltou32(x:bool) -> (y:u32)``
- ``u32tou64(x:u32) -> (y:u64)``
- ``u64tou32(x:u64) -> (y:u32)``
- etc. (TBD)

Low-level Functions
-------------------

The following functions must be available:

+---------------------------------------------------------------------------------------------------------------+
| *Arithmetics*                                                                                                 |
+---------------------------------------------------------------------------------------------------------------+
| addu256(x:u256, y:u256) -> z:u256           | x + y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| subu256(x:u256, y:u256) -> z:u256           | x - y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| mulu256(x:u256, y:u256) -> z:u256           | x * y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| divu256(x:u256, y:u256) -> z:u256           | x / y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| divs256(x:s256, y:s256) -> z:s256           | x / y, for signed numbers in two's complement                   |
+---------------------------------------------------------------------------------------------------------------+
| modu256(x:u256, y:u256) -> z:u256           | x % y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| mods256(x:s256, y:s256) -> z:s256           | x % y, for signed numbers in two's complement                   |
+---------------------------------------------------------------------------------------------------------------+
| signextendu256(i:u256, x:u256) -> z:u256    | sign extend from (i*8+7)th bit counting from least significant  |
+---------------------------------------------------------------------------------------------------------------+
| expu256(x:u256, y:u256) -> z:u256           | x to the power of y                                             |
+---------------------------------------------------------------------------------------------------------------+
| addmodu256(x:u256, y:u256, m:u256) -> z:u256| (x + y) % m with arbitrary precision arithmetics                |
+---------------------------------------------------------------------------------------------------------------+
| mulmodu256(x:u256, y:u256, m:u256) -> z:u256| (x * y) % m with arbitrary precision arithmetics                |
+---------------------------------------------------------------------------------------------------------------+
| ltu256(x:u256, y:u256) -> z:bool            | 1 if x < y, 0 otherwise                                         |
+---------------------------------------------------------------------------------------------------------------+
| gtu256(x:u256, y:u256) -> z:bool            | 1 if x > y, 0 otherwise                                         |
+---------------------------------------------------------------------------------------------------------------+
| sltu256(x:s256, y:s256) -> z:bool           | 1 if x < y, 0 otherwise, for signed numbers in two's complement |
+---------------------------------------------------------------------------------------------------------------+
| sgtu256(x:s256, y:s256) -> z:bool           | 1 if x > y, 0 otherwise, for signed numbers in two's complement |
+---------------------------------------------------------------------------------------------------------------+
| equ256(x:u256, y:u256) -> z:bool            | 1 if x == y, 0 otherwise                                        |
+---------------------------------------------------------------------------------------------------------------+
| notu256(x:u256) -> z:u256                   | ~x, every bit of x is negated                                   |
+---------------------------------------------------------------------------------------------------------------+
| andu256(x:u256, y:u256) -> z:u256           | bitwise and of x and y                                          |
+---------------------------------------------------------------------------------------------------------------+
| oru256(x:u256, y:u256) -> z:u256            | bitwise or of x and y                                           |
+---------------------------------------------------------------------------------------------------------------+
| xoru256(x:u256, y:u256) -> z:u256           | bitwise xor of x and y                                          |
+---------------------------------------------------------------------------------------------------------------+
| shlu256(x:u256, y:u256) -> z:u256           | logical left shift of x by y                                    |
+---------------------------------------------------------------------------------------------------------------+
| shru256(x:u256, y:u256) -> z:u256           | logical right shift of x by y                                   |
+---------------------------------------------------------------------------------------------------------------+
| saru256(x:u256, y:u256) -> z:u256           | arithmetic right shift of x by y                                |
+---------------------------------------------------------------------------------------------------------------+
| byte(n:u256, x:u256) -> v:u256              | nth byte of x, where the most significant byte is the 0th byte  |
| Cannot this be just replaced by and256(shr256(n, x), 0xff) and let it be optimised out by the EVM backend?    |
+---------------------------------------------------------------------------------------------------------------+
| *Memory and storage*                                                                                          |
+---------------------------------------------------------------------------------------------------------------+
| mload(p:u256) -> v:u256                     | mem[p..(p+32))                                                  |
+---------------------------------------------------------------------------------------------------------------+
| mstore(p:u256, v:u256)                      | mem[p..(p+32)) := v                                             |
+---------------------------------------------------------------------------------------------------------------+
| mstore8(p:u256, v:u256)                     | mem[p] := v & 0xff    - only modifies a single byte             |
+---------------------------------------------------------------------------------------------------------------+
| sload(p:u256) -> v:u256                     | storage[p]                                                      |
+---------------------------------------------------------------------------------------------------------------+
| sstore(p:u256, v:u256)                      | storage[p] := v                                                 |
+---------------------------------------------------------------------------------------------------------------+
| msize() -> size:u256                        | size of memory, i.e. largest accessed memory index, albeit due  |
|                                             | due to the memory extension function, which extends by words,   |
|                                             | this will always be a multiple of 32 bytes                      |
+---------------------------------------------------------------------------------------------------------------+
| *Execution control*                                                                                           |
+---------------------------------------------------------------------------------------------------------------+
| create(v:u256, p:u256, s:u256)              | create new contract with code mem[p..(p+s)) and send v wei      |
|                                             | and return the new address                                      |
+---------------------------------------------------------------------------------------------------------------+
| call(g:u256, a:u256, v:u256, in:u256,       | call contract at address a with input mem[in..(in+insize))      |
| insize:u256, out:u256,                      | providing g gas and v wei and output area                       |
| outsize:u256)                               | mem[out..(out+outsize)) returning 0 on error (eg. out of gas)   |
| -> r:u256                                   | and 1 on success                                                |
+---------------------------------------------------------------------------------------------------------------+
| callcode(g:u256, a:u256, v:u256, in:u256,   | identical to ``call`` but only use the code from a              |
| insize:u256, out:u256,                      | and stay in the context of the                                  |
| outsize:u256) -> r:u256                     | current contract otherwise                                      |
+---------------------------------------------------------------------------------------------------------------+
| delegatecall(g:u256, a:u256, in:u256,       | identical to ``callcode``,                                      |
| insize:u256, out:u256,                      | but also keep ``caller``                                        |
| outsize:u256) -> r:u256                     | and ``callvalue``                                               |
+---------------------------------------------------------------------------------------------------------------+
| stop()                                      | stop execution, identical to return(0,0)                        |
| Perhaps it would make sense retiring this as it equals to return(0,0). It can be an optimisation by the EVM   |
| backend.                                                                                                      |
+---------------------------------------------------------------------------------------------------------------+
| abort()                                     | abort (equals to invalid instruction on EVM)                    |
+---------------------------------------------------------------------------------------------------------------+
| return(p:u256, s:u256)                      | end execution, return data mem[p..(p+s))                        |
+---------------------------------------------------------------------------------------------------------------+
| revert(p:u256, s:u256)                      | end execution, revert state changes, return data mem[p..(p+s))  |
+---------------------------------------------------------------------------------------------------------------+
| selfdestruct(a:u256)                        | end execution, destroy current contract and send funds to a     |
+---------------------------------------------------------------------------------------------------------------+
| log0(p:u256, s:u256)                        | log without topics and data mem[p..(p+s))                       |
+---------------------------------------------------------------------------------------------------------------+
| log1(p:u256, s:u256, t1:u256)               | log with topic t1 and data mem[p..(p+s))                        |
+---------------------------------------------------------------------------------------------------------------+
| log2(p:u256, s:u256, t1:u256, t2:u256)      | log with topics t1, t2 and data mem[p..(p+s))                   |
+---------------------------------------------------------------------------------------------------------------+
| log3(p:u256, s:u256, t1:u256, t2:u256,      | log with topics t, t2, t3 and data mem[p..(p+s))                |
| t3:u256)                                    |                                                                 |
+---------------------------------------------------------------------------------------------------------------+
| log4(p:u256, s:u256, t1:u256, t2:u256,      | log with topics t1, t2, t3, t4 and data mem[p..(p+s))           |
| t3:u256, t4:u256)                           |                                                                 |
+---------------------------------------------------------------------------------------------------------------+
| *State queries*                                                                                               |
+---------------------------------------------------------------------------------------------------------------+
| blockcoinbase() -> address:u256             | current mining beneficiary                                      |
+---------------------------------------------------------------------------------------------------------------+
| blockdifficulty() -> difficulty:u256        | difficulty of the current block                                 |
+---------------------------------------------------------------------------------------------------------------+
| blockgaslimit() -> limit:u256               | block gas limit of the current block                            |
+---------------------------------------------------------------------------------------------------------------+
| blockhash(b:u256) -> hash:u256              | hash of block nr b - only for last 256 blocks excluding current |
+---------------------------------------------------------------------------------------------------------------+
| blocknumber() -> block:u256                 | current block number                                            |
+---------------------------------------------------------------------------------------------------------------+
| blocktimestamp() -> timestamp:u256          | timestamp of the current block in seconds since the epoch       |
+---------------------------------------------------------------------------------------------------------------+
| txorigin() -> address:u256                  | transaction sender                                              |
+---------------------------------------------------------------------------------------------------------------+
| txgasprice() -> price:u256                  | gas price of the transaction                                    |
+---------------------------------------------------------------------------------------------------------------+
| gasleft() -> gas:u256                       | gas still available to execution                                |
+---------------------------------------------------------------------------------------------------------------+
| balance(a:u256) -> v:u256                   | wei balance at address a                                        |
+---------------------------------------------------------------------------------------------------------------+
| this() -> address:u256                      | address of the current contract / execution context             |
+---------------------------------------------------------------------------------------------------------------+
| caller() -> address:u256                    | call sender (excluding delegatecall)                            |
+---------------------------------------------------------------------------------------------------------------+
| callvalue() -> v:u256                       | wei sent together with the current call                         |
+---------------------------------------------------------------------------------------------------------------+
| calldataload(p:u256) -> v:u256              | call data starting from position p (32 bytes)                   |
+---------------------------------------------------------------------------------------------------------------+
| calldatasize() -> v:u256                    | size of call data in bytes                                      |
+---------------------------------------------------------------------------------------------------------------+
| calldatacopy(t:u256, f:u256, s:u256)        | copy s bytes from calldata at position f to mem at position t   |
+---------------------------------------------------------------------------------------------------------------+
| codesize() -> size:u256                     | size of the code of the current contract / execution context    |
+---------------------------------------------------------------------------------------------------------------+
| codecopy(t:u256, f:u256, s:u256)            | copy s bytes from code at position f to mem at position t       |
+---------------------------------------------------------------------------------------------------------------+
| extcodesize(a:u256) -> size:u256            | size of the code at address a                                   |
+---------------------------------------------------------------------------------------------------------------+
| extcodecopy(a:u256, t:u256, f:u256, s:u256) | like codecopy(t, f, s) but take code at address a               |
+---------------------------------------------------------------------------------------------------------------+
| *Others*                                                                                                      |
+---------------------------------------------------------------------------------------------------------------+
| discardu256(unused:u256)                    | discard value                                                   |
+---------------------------------------------------------------------------------------------------------------+
| splitu256tou64(x:u256) -> (x1:u64, x2:u64,  | split u256 to four u64's                                        |
|                            x3:u64, x4:u64)  |                                                                 |
+---------------------------------------------------------------------------------------------------------------+
| combineu64tou256(x1:u64, x2:u64, x3:u64,    | combine four u64's into a single u256                           |
|                  x4:u64) -> (x:u256)        |                                                                 |
+---------------------------------------------------------------------------------------------------------------+
| sha3(p:u256, s:u256) -> v:u256              | keccak(mem[p...(p+s)))                                          |
+---------------------------------------------------------------------------------------------------------------+

Backends
--------

Backends or targets are the translators from JULIA to a specific bytecode. Each of the backends can expose functions
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
