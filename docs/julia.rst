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

JULIA in itself does not even provide operators. If the EVM is targeted,
opcodes will be available as built-in functions, but they can be reimplemented
if the backend changes.

The following example program assumes that the EVM opcodes ``mul``, ``div``
and ``mod`` are available either natively or as functions and computes exponentiation.

.. code::

    {
        function power(base, exponent) -> (result)
        {
            switch exponent
            0: { result := 1 }
            1: { result := base }
            default:
            {
                result := power(mul(base, base), div(exponent, 2))
                switch mod(exponent, 2)
                    1: { result := mul(base, result) }
            }
        }
    }

It is also possible to implement the same function using a for-loop
instead of recursion. Here, we need the EVM opcodes ``lt`` (less-than)
and ``add`` to be available.

.. code::

    {
        function power(base, exponent) -> (result)
        {
            result := 1
            for { let i := 0 } lt(i, exponent) { i := add(i, 1) }
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
