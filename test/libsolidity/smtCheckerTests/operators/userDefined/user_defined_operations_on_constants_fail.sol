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
    I16 constant MINUS_TWO = I16.wrap(-2);
    I16 constant ZERO = I16.wrap(0);
    I16 constant ONE = I16.wrap(1);
    I16 constant TWO = I16.wrap(2);
    I16 constant THREE = I16.wrap(3);
    I16 constant FOUR = I16.wrap(4);

    function testBitwise() public pure {
        assert(ONE | TWO == FOUR); // should fail
        assert(ONE & THREE == FOUR); // should fail
        assert(TWO ^ TWO == FOUR); // should fail
        assert(~ONE == FOUR); // should fail
    }

    function testArithmetic() public pure {
        assert(TWO + THREE == FOUR); // should fail
        assert(TWO - TWO == FOUR); // should fail
        assert(-TWO == FOUR); // should fail
        assert(TWO * THREE == FOUR); // should fail
        assert(TWO / TWO == FOUR); // should fail
        assert(TWO % TWO == FOUR); // should fail
    }

    function testComparison() public pure {
        assert(!(TWO == TWO)); // should fail
        assert(TWO != TWO); // should fail
        assert(TWO < TWO); // should fail
        assert(TWO > TWO); // should fail
        assert(!(TWO <= TWO)); // should fail
        assert(!(TWO >= TWO)); // should fail
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (2012-2037): CHC: Assertion violation happens here.
// Warning 6328: (2062-2089): CHC: Assertion violation happens here.
// Warning 6328: (2114-2139): CHC: Assertion violation happens here.
// Warning 6328: (2164-2184): CHC: Assertion violation happens here.
// Warning 6328: (2260-2287): CHC: Assertion violation happens here.
// Warning 6328: (2312-2337): CHC: Assertion violation happens here.
// Warning 6328: (2362-2382): CHC: Assertion violation happens here.
// Warning 6328: (2407-2434): CHC: Assertion violation happens here.
// Warning 6328: (2459-2484): CHC: Assertion violation happens here.
// Warning 6328: (2509-2534): CHC: Assertion violation happens here.
// Warning 6328: (2610-2631): CHC: Assertion violation happens here.
// Warning 6328: (2656-2674): CHC: Assertion violation happens here.
// Warning 6328: (2699-2716): CHC: Assertion violation happens here.
// Warning 6328: (2741-2758): CHC: Assertion violation happens here.
// Warning 6328: (2783-2804): CHC: Assertion violation happens here.
// Warning 6328: (2829-2850): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
