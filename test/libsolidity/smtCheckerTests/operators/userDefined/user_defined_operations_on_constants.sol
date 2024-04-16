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
        assert(ONE | TWO == THREE);
        assert(ONE & THREE == ONE);
        assert(TWO ^ TWO == ZERO);
        assert(~ONE == MINUS_TWO);
    }

    function testArithmetic() public pure {
        assert(TWO + TWO == FOUR);
        assert(TWO - TWO == ZERO);
        assert(-TWO == MINUS_TWO);
        assert(TWO * TWO == FOUR);
        assert(TWO / TWO == ONE);
        assert(TWO % TWO == ZERO);
    }

    function testComparison() public pure {
        assert(TWO == TWO);
        assert(!(TWO != TWO));
        assert(!(TWO < TWO));
        assert(!(TWO > TWO));
        assert(TWO <= TWO);
        assert(TWO >= TWO);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 21 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

