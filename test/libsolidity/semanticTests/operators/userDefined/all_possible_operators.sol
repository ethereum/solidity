type Int is int8;
using {
    bitor as |, bitand as &, bitxor as ^, bitnot as ~,
    add as +, sub as -, unsub as -, mul as *, div as /, mod as %,
    eq as ==, noteq as !=, lt as <, gt as >, leq as <=, geq as >=
} for Int global;

function bitor(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) | Int.unwrap(y)); }
function bitand(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) & Int.unwrap(y)); }
function bitxor(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) ^ Int.unwrap(y)); }
function bitnot(Int x) pure returns (Int) { return Int.wrap(~Int.unwrap(x)); }

function add(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) + Int.unwrap(y)); }
function sub(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) - Int.unwrap(y)); }
function unsub(Int x) pure returns (Int) { return Int.wrap(-Int.unwrap(x)); }
function mul(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) * Int.unwrap(y)); }
function div(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) / Int.unwrap(y)); }
function mod(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) % Int.unwrap(y)); }

function eq(Int x, Int y) pure returns (bool) { return Int.unwrap(x) == Int.unwrap(y); }
function noteq(Int x, Int y) pure returns (bool) { return Int.unwrap(x) != Int.unwrap(y); }
function lt(Int x, Int y) pure returns (bool) { return Int.unwrap(x) < Int.unwrap(y); }
function gt(Int x, Int y) pure returns (bool) { return Int.unwrap(x) > Int.unwrap(y); }
function leq(Int x, Int y) pure returns (bool) { return Int.unwrap(x) <= Int.unwrap(y); }
function geq(Int x, Int y) pure returns (bool) { return Int.unwrap(x) >= Int.unwrap(y); }

contract C {
    Int constant ZERO = Int.wrap(0);
    Int constant ONE = Int.wrap(1);
    Int constant TWO = Int.wrap(2);
    Int constant THREE = Int.wrap(3);
    Int constant SIX = Int.wrap(6);

    function testBitwise() public pure {
        assert(Int.unwrap(ONE | TWO) == 3);
        assert(Int.unwrap(ONE | ZERO) == 1);

        assert(Int.unwrap(ONE & THREE) == 1);
        assert(Int.unwrap(ONE & ONE) == 1);

        assert(Int.unwrap(TWO ^ TWO) == 0);
        assert(Int.unwrap(TWO ^ ONE) == 3);

        assert(Int.unwrap(~ZERO) == -1);
        assert(Int.unwrap(~ONE) == -2);
        assert(Int.unwrap(~TWO) == -3);
    }

    function testArithmetic() public pure {
        assert(Int.unwrap(ONE + TWO) == 3);
        assert(Int.unwrap(ONE + ZERO) == 1);

        assert(Int.unwrap(TWO - ONE) == 1);
        assert(Int.unwrap(THREE - THREE) == 0);

        assert(Int.unwrap(-TWO) == -2);
        assert(Int.unwrap(-ZERO) == 0);

        assert(Int.unwrap(ONE * ONE) == 1);
        assert(Int.unwrap(THREE * TWO) == 6);

        assert(Int.unwrap(SIX / TWO) == 3);
        assert(Int.unwrap(THREE / TWO) == 1);

        assert(Int.unwrap(SIX % TWO) == 0);
        assert(Int.unwrap(THREE % TWO) == 1);
    }

    function testComparison() public pure {
        assert((ONE == ONE) == true);
        assert((ONE == TWO) == false);

        assert((ONE != ONE) == false);
        assert((ONE != TWO) == true);

        assert((ONE < TWO) == true);
        assert((TWO < ONE) == false);

        assert((ONE <= TWO) == true);
        assert((TWO <= ONE) == false);

        assert((ONE > TWO) == false);
        assert((TWO > ONE) == true);

        assert((ONE >= TWO) == false);
        assert((TWO >= ONE) == true);
    }
}
// ----
// testBitwise() ->
// testArithmetic() ->
// testComparison() ->
