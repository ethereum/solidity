# NestedArrayFunctionCallDecoder

## buggy

function f() pure returns (uint[2][2]) { }

--

function f() returns (uint[2][2] a) { }

--

function f() returns (uint x, uint[200][2] a) { }

--

function f() returns (uint[200][2] a, uint x) { }

--

function f() returns (uint[200][2] a, uint x);

--

function f() returns (
    uint
    [
    200
    ]
    [2]
    a, uint x);

--

function f() returns (
    uint
    [
    ContractName.ConstantName
    ]
    [2]
    a, uint x);

## fine

function f() returns (uint[2]) { }

--

function f() public pure returns (uint[2][] a) { }

--

function f() public pure returns (uint[ 2 ] [ ]  a) { }

--

function f() public pure  returns (uint x, uint[] a) { }

--

function f(uint[2][2]) { }

--

function f() m(uint[2][2]) { }

--

function f() returns (uint, uint) { uint[2][2] memory x; }
