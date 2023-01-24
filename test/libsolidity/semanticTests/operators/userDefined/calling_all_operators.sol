type Int is int8;
using {
    bitor as |, bitand as &, bitxor as ^, bitnot as ~, shl as <<, sar as >>,
    add as +, sub as -, unsub as -, mul as *, div as /, mod as %, exp as **,
    eq as ==, noteq as !=, lt as <, gt as >, leq as <=, geq as >=,
    not as !
} for Int;

function bitor(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) | Int.unwrap(y)); }
function bitand(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) & Int.unwrap(y)); }
function bitxor(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) ^ Int.unwrap(y)); }
function bitnot(Int x) pure returns (Int) { return Int.wrap(~Int.unwrap(x)); }
function shl(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) << uint8(Int.unwrap(y))); }
function sar(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) >> uint8(Int.unwrap(y))); }

function add(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) + Int.unwrap(y)); }
function sub(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) - Int.unwrap(y)); }
function unsub(Int x) pure returns (Int) { return Int.wrap(-Int.unwrap(x)); }
function mul(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) * Int.unwrap(y)); }
function div(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) / Int.unwrap(y)); }
function mod(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) % Int.unwrap(y)); }
function exp(Int x, Int y) pure returns (Int) { return Int.wrap(Int.unwrap(x) ** uint8(Int.unwrap(y))); }

function eq(Int x, Int y) pure returns (bool) { return Int.unwrap(x) == Int.unwrap(y); }
function noteq(Int x, Int y) pure returns (bool) { return Int.unwrap(x) != Int.unwrap(y); }
function lt(Int x, Int y) pure returns (bool) { return Int.unwrap(x) < Int.unwrap(y); }
function gt(Int x, Int y) pure returns (bool) { return Int.unwrap(x) > Int.unwrap(y); }
function leq(Int x, Int y) pure returns (bool) { return Int.unwrap(x) <= Int.unwrap(y); }
function geq(Int x, Int y) pure returns (bool) { return Int.unwrap(x) >= Int.unwrap(y); }

function not(Int x) pure returns (Int) { return Int.unwrap(x) == 0 ? Int.wrap(1) : Int.wrap(0); }

contract C {
    Int constant ZERO = Int.wrap(0);
    Int constant ONE = Int.wrap(1);
    Int constant TWO = Int.wrap(2);
    Int constant THREE = Int.wrap(3);
    Int constant FOUR = Int.wrap(4);
    Int constant SIX = Int.wrap(6);

    function test_bitwise() public pure {
        assert(Int.unwrap(ONE | TWO) == 3);
        assert(Int.unwrap(ONE | ZERO) == 1);

        assert(Int.unwrap(ONE & THREE) == 1);
        assert(Int.unwrap(ONE & ONE) == 1);

        assert(Int.unwrap(TWO ^ TWO) == 0);
        assert(Int.unwrap(TWO ^ ONE) == 3);

        assert(Int.unwrap(~ZERO) == -1);
        assert(Int.unwrap(~ONE) == -2);
        assert(Int.unwrap(~TWO) == -3);

        assert(Int.unwrap(ONE << ONE) == 2);
        assert(Int.unwrap(ONE << TWO) == 4);

        assert(Int.unwrap(TWO >> ONE) == 1);
        assert(Int.unwrap(FOUR >> TWO) == 1);
    }

    function test_arithmetic() public pure {
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

        assert(Int.unwrap(ONE ** SIX) == 1);
        assert(Int.unwrap(TWO ** TWO) == 4);
    }

    function test_comparison() public pure {
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

    function test_boolean() public pure {
        assert(Int.unwrap(!ZERO) == 1);
        assert(Int.unwrap(!ONE) == 0);
        assert(Int.unwrap(!TWO) == 0);
    }
}
// ----
// test_bitwise() ->
// test_arithmetic() ->
// test_comparison() ->
// test_boolean() ->
