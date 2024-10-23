type I16 is int16;
using {
    bitor as |, bitand as &, bitxor as ^, bitnot as ~,
    add as +, sub as -, unsub as -, mul as *, div as /, mod as %,
    eq as ==, noteq as !=, lt as <, gt as >, leq as <=, geq as >=
} for I16 global;

function bitor(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) | I16.unwrap(y)); }
function bitand(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) & I16.unwrap(y)); }
function bitxor(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) ^ I16.unwrap(y)); }
function bitnot(I16 x) pure returns (I16) { return I16.wrap(~I16.unwrap(x)); }

function add(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) + I16.unwrap(y)); }
function sub(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) - I16.unwrap(y)); }
function unsub(I16 x) pure returns (I16) { return I16.wrap(-I16.unwrap(x)); }
function mul(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) * I16.unwrap(y)); }
function div(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) / I16.unwrap(y)); }
function mod(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) % I16.unwrap(y)); }


function eq(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) == I16.unwrap(y); }
function noteq(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) != I16.unwrap(y); }
function lt(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) < I16.unwrap(y); }
function gt(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) > I16.unwrap(y); }
function leq(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) <= I16.unwrap(y); }
function geq(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) >= I16.unwrap(y); }

contract C {
    function testBitwise(I16 x, I16 y) public pure {
        assert(x | y == bitor(x, y)); // should hold
        assert(x & y == bitand(x, y)); // should hold
        assert(x ^ y == bitxor(x, y)); // should hold
        assert(~x == bitnot(x)); // should hold
    }

    function testArithmetic(I16 x, I16 y) public pure {
        assert(x + y == add(x, y));
        assert(x - y == sub(x, y));
        assert(-x == unsub(x));
        assert(x * y == mul(x, y));
        assert(x / y == div(x, y));
        assert(x % y == mod(x, y));
    }

    function testComparison(I16 x, I16 y) public pure {
        assert((x == y) == eq(x, y)); // should hold
        assert((x != y) == noteq(x, y)); // should hold
        assert((x < y) == lt(x, y)); // should hold
        assert((x > y) == gt(x, y)); // should hold
        assert((x <= y) == leq(x, y)); // should hold
        assert((x >= y) == geq(x, y)); // should hold
    }
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// SMTTargets: assert
// ----
// Warning 6328: (2209-2235): CHC: Assertion violation might happen here.
// Warning 6328: (2245-2271): CHC: Assertion violation might happen here.
// Info 1391: CHC: 14 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
