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

Low-level Functions
-------------------

The following functions must be available:

+---------------------------------------------------------------------------------------------------------------+
| *Arithmetics*                                                                                                 |
+---------------------------------------------------------------------------------------------------------------+
| add256(x:256, y:256) -> z:256               | x + y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| sub256(x:256, y:256) -> z:256               | x - y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| mul256(x:256, y:256) -> z:256               | x * y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| div256(x:256, y:256) -> z:256               | x / y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| sdiv256(x:256, y:256) -> z:256              | x / y, for signed numbers in two's complement                   |
+---------------------------------------------------------------------------------------------------------------+
| mod256(x:256, y:256) -> z:256               | x % y                                                           |
+---------------------------------------------------------------------------------------------------------------+
| smod256(x:256, y:256) -> z:256              | x % y, for signed numbers in two's complement                   |
+---------------------------------------------------------------------------------------------------------------+
| signextend256(i:256, x:256) -> z:256        | sign extend from (i*8+7)th bit counting from least significant  |
+---------------------------------------------------------------------------------------------------------------+
| exp256(x:256, y:256) -> z:256               | x to the power of y                                             |
+---------------------------------------------------------------------------------------------------------------+
| addmod256(x:256, y:256, m:256) -> z:256     | (x + y) % m with arbitrary precision arithmetics                |
+---------------------------------------------------------------------------------------------------------------+
| mulmod256(x:256, y:256, m:256) -> z:256     | (x * y) % m with arbitrary precision arithmetics                |
+---------------------------------------------------------------------------------------------------------------+
| lt256(x:256, y:256) -> z:bool               | 1 if x < y, 0 otherwise                                         |
+---------------------------------------------------------------------------------------------------------------+
| gt256(x:256, y:256) -> z:bool               | 1 if x > y, 0 otherwise                                         |
+---------------------------------------------------------------------------------------------------------------+
| slt256(x:256, y:256) -> z:bool              | 1 if x < y, 0 otherwise, for signed numbers in two's complement |
+---------------------------------------------------------------------------------------------------------------+
| sgt256(x:256, y:256) -> z:bool              | 1 if x > y, 0 otherwise, for signed numbers in two's complement |
+---------------------------------------------------------------------------------------------------------------+
| eq256(x:256, y:256) -> z:bool               | 1 if x == y, 0 otherwise                                        |
+---------------------------------------------------------------------------------------------------------------+
| not256(x:256) -> z:256                      | ~x, every bit of x is negated                                   |
+---------------------------------------------------------------------------------------------------------------+
| and256(x:256, y:256) -> z:256               | bitwise and of x and y                                          |
+---------------------------------------------------------------------------------------------------------------+
| or256(x:256, y:256) -> z:256                | bitwise or of x and y                                           |
+---------------------------------------------------------------------------------------------------------------+
| xor256(x:256, y:256) -> z:256               | bitwise xor of x and y                                          |
+---------------------------------------------------------------------------------------------------------------+
| shl256(x:256, y:256) -> z:256               | logical left shift of x by y                                    |
+---------------------------------------------------------------------------------------------------------------+
| shr256(x:256, y:256) -> z:256               | logical right shift of x by y                                   |
+---------------------------------------------------------------------------------------------------------------+
| sar256(x:256, y:256) -> z:256               | arithmetic right shift of x by y                                |
+---------------------------------------------------------------------------------------------------------------+
| byte(n:256, x:256) -> v:256                 | nth byte of x, where the most significant byte is the 0th byte  |
| Cannot this be just replaced by and256(shr256(n, x), 0xff) and let it be optimised out by the EVM backend?    |
+---------------------------------------------------------------------------------------------------------------+
| *Memory and storage*                                                                                          |
+---------------------------------------------------------------------------------------------------------------+
| mload(p:256) -> v:256                       | mem[p..(p+32))                                                  |
+---------------------------------------------------------------------------------------------------------------+
| mstore(p:256, v:256)                        | mem[p..(p+32)) := v                                             |
+---------------------------------------------------------------------------------------------------------------+
| mstore8(p:256, v:256)                       | mem[p] := v & 0xff    - only modifies a single byte             |
+---------------------------------------------------------------------------------------------------------------+
| sload(p:256) -> v:256                       | storage[p]                                                      |
+---------------------------------------------------------------------------------------------------------------+
| sstore(p:256, v:256)                        | storage[p] := v                                                 |
+---------------------------------------------------------------------------------------------------------------+
| msize() -> size:256                         | size of memory, i.e. largest accessed memory index, albeit due  |
|                                             | due to the memory extension function, which extends by words,   |
|                                             | this will always be a multiple of 32 bytes                      |
+---------------------------------------------------------------------------------------------------------------+
| *Execution control*                                                                                           |
+---------------------------------------------------------------------------------------------------------------+
| create(v:256, p:256, s:256)                 | create new contract with code mem[p..(p+s)) and send v wei      |
|                                             | and return the new address                                      |
+---------------------------------------------------------------------------------------------------------------+
| call(g:256, a:256, v:256, in:256,           | call contract at address a with input mem[in..(in+insize))      |
| insize:256, out:256, outsize:256) -> r:256  | providing g gas and v wei and output area                       |
|                                             | mem[out..(out+outsize)) returning 0 on error (eg. out of gas)   |
|                                             | and 1 on success                                                |
+---------------------------------------------------------------------------------------------------------------+
| callcode(g:256, a:256, v:256, in:256,       | identical to `call` but only use the code from a and stay       |
| insize:256, out:256, outsize:256) -> r:256  | in the context of the current contract otherwise                |
+---------------------------------------------------------------------------------------------------------------+
| delegatecall(g:256, a:256, in:256,          | identical to `callcode` but also keep ``caller``                |
| insize:256, out:256, outsize:256) -> r:256  | and ``callvalue``                                               |
+---------------------------------------------------------------------------------------------------------------+
| stop()                                      | stop execution, identical to return(0,0)                        |
| Perhaps it would make sense retiring this as it equals to return(0,0). It can be an optimisation by the EVM   |
| backend.                                                                                                      |
+---------------------------------------------------------------------------------------------------------------+
| abort()                                     | abort (equals to invalid instruction on EVM)                    |
+---------------------------------------------------------------------------------------------------------------+
| return(p:256, s:256)                        | end execution, return data mem[p..(p+s))                        |
+---------------------------------------------------------------------------------------------------------------+
| revert(p:256, s:256)                        | end execution, revert state changes, return data mem[p..(p+s))  |
+---------------------------------------------------------------------------------------------------------------+
| selfdestruct(a:256)                         | end execution, destroy current contract and send funds to a     |
+---------------------------------------------------------------------------------------------------------------+
| log0(p:256, s:256)                          | log without topics and data mem[p..(p+s))                       |
+---------------------------------------------------------------------------------------------------------------+
| log1(p:256, s:256, t1:256)                  | log with topic t1 and data mem[p..(p+s))                        |
+---------------------------------------------------------------------------------------------------------------+
| log2(p:256, s:256, t1:256, t2:256)          | log with topics t1, t2 and data mem[p..(p+s))                   |
+---------------------------------------------------------------------------------------------------------------+
| log3(p:256, s:256, t1:256, t2:256,          | log with topics t, t2, t3 and data mem[p..(p+s))                |
| t3:256)                                     |                                                                 |
+---------------------------------------------------------------------------------------------------------------+
| log4(p:256, s:256, t1:256, t2:256,          | log with topics t1, t2, t3, t4 and data mem[p..(p+s))           |
| t3:256, t4:256)                             |                                                                 |
+---------------------------------------------------------------------------------------------------------------+
| *State queries*                                                                                               |
+---------------------------------------------------------------------------------------------------------------+
| blockcoinbase() -> address:256              | current mining beneficiary                                      |
+---------------------------------------------------------------------------------------------------------------+
| blockdifficulty() -> difficulty:256         | difficulty of the current block                                 |
+---------------------------------------------------------------------------------------------------------------+
| blockgaslimit() -> limit:256                | block gas limit of the current block                            |
+---------------------------------------------------------------------------------------------------------------+
| blockhash(b:256) -> hash:256                | hash of block nr b - only for last 256 blocks excluding current |
+---------------------------------------------------------------------------------------------------------------+
| blocknumber() -> block:256                  | current block number                                            |
+---------------------------------------------------------------------------------------------------------------+
| blocktimestamp() -> timestamp:256           | timestamp of the current block in seconds since the epoch       |
+---------------------------------------------------------------------------------------------------------------+
| txorigin() -> address:256                   | transaction sender                                              |
+---------------------------------------------------------------------------------------------------------------+
| txgasprice() -> price:256                   | gas price of the transaction                                    |
+---------------------------------------------------------------------------------------------------------------+
| gasleft() -> gas:256                        | gas still available to execution                                |
+---------------------------------------------------------------------------------------------------------------+
| balance(a:256) -> v:256                     | wei balance at address a                                        |
+---------------------------------------------------------------------------------------------------------------+
| this() -> address:256                       | address of the current contract / execution context             |
+---------------------------------------------------------------------------------------------------------------+
| caller() -> address:256                     | call sender (excluding delegatecall)                            |
+---------------------------------------------------------------------------------------------------------------+
| callvalue() -> v:256                        | wei sent together with the current call                         |
+---------------------------------------------------------------------------------------------------------------+
| calldataload(p:256) -> v:256                | call data starting from position p (32 bytes)                   |
+---------------------------------------------------------------------------------------------------------------+
| calldatasize() -> v:256                     | size of call data in bytes                                      |
+---------------------------------------------------------------------------------------------------------------+
| calldatacopy(t:256, f:256, s:256)           | copy s bytes from calldata at position f to mem at position t   |
+---------------------------------------------------------------------------------------------------------------+
| codesize() -> size:256                      | size of the code of the current contract / execution context    |
+---------------------------------------------------------------------------------------------------------------+
| codecopy(t:256, f:256, s:256)               | copy s bytes from code at position f to mem at position t       |
+---------------------------------------------------------------------------------------------------------------+
| extcodesize(a:256) -> size:256              | size of the code at address a                                   |
+---------------------------------------------------------------------------------------------------------------+
| extcodecopy(a:256, t:256, f:256, s:256)     | like codecopy(t, f, s) but take code at address a               |
+---------------------------------------------------------------------------------------------------------------+
| *Others*                                                                                                      |
+---------------------------------------------------------------------------------------------------------------+
| discard256(unused:256)                      | discard value                                                   |
+---------------------------------------------------------------------------------------------------------------+
| sha3(p:256, s:256) -> v:256                 | keccak(mem[p...(p+s)))                                          |
+---------------------------------------------------------------------------------------------------------------+
