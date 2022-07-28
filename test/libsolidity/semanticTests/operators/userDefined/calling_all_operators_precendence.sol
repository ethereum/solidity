type Int is int64;
using {
    bitor as |, bitand as &, bitxor as ^, bitnot as ~, shl as <<, sar as >>,
    add as +, sub as -, unsub as -, mul as *, div as /, mod as %, exp as **,
    eq as ==, noteq as !=,
    not as !
} for Int;

function uw(Int x) pure returns (int64) { return Int.unwrap(x); }
function w(int64 x) pure returns (Int) { return Int.wrap(x); }

function bitor(Int x, Int y) pure returns (Int) { return w(uw(x) | uw(y)); }
function bitand(Int x, Int y) pure returns (Int) { return w(uw(x) & uw(y)); }
function bitxor(Int x, Int y) pure returns (Int) { return w(uw(x) ^ uw(y)); }
function bitnot(Int x) pure returns (Int) { return w(~uw(x)); }
function shl(Int x, Int y) pure returns (Int) { return w(uw(x) << uint64(uw(y))); }
function sar(Int x, Int y) pure returns (Int) { return w(uw(x) >> uint64(uw(y))); }

function add(Int x, Int y) pure returns (Int) { return w(uw(x) + uw(y)); }
function sub(Int x, Int y) pure returns (Int) { return w(uw(x) - uw(y)); }
function unsub(Int x) pure returns (Int) { return w(-uw(x)); }
function mul(Int x, Int y) pure returns (Int) { return w(uw(x) * uw(y)); }
function div(Int x, Int y) pure returns (Int) { return w(uw(x) / uw(y)); }
function mod(Int x, Int y) pure returns (Int) { return w(uw(x) % uw(y)); }
function exp(Int x, Int y) pure returns (Int) { return w(uw(x) ** uint64(uw(y))); }

function eq(Int x, Int y) pure returns (bool) { return uw(x) == uw(y); }
function noteq(Int x, Int y) pure returns (bool) { return uw(x) != uw(y); }

function not(Int x) pure returns (Int) { return w((uw(x) == 0 ? int64(1) : int64(0))); }

contract C {
    Int constant I0 = Int.wrap(0);
    Int constant I1 = Int.wrap(1);
    Int constant I2 = Int.wrap(2);
    Int constant I3 = Int.wrap(3);
    Int constant I4 = Int.wrap(4);
    Int constant I5 = Int.wrap(5);
    Int constant I6 = Int.wrap(6);
    Int constant I7 = Int.wrap(7);
    Int constant I10 = Int.wrap(10);
    Int constant I13 = Int.wrap(13);
    Int constant I15 = Int.wrap(15);
    Int constant I20 = Int.wrap(20);
    Int constant I128 = Int.wrap(128);

    function test_bitwise() public pure {
        assert(Int.unwrap(I0 & I0 | I1) == (0 & 0 | 1));
        assert(Int.unwrap(I0 & I0 | I1) == ((0 & 0) | 1));
    }

    function test_bitwise_arithmetic() public pure {
        assert(Int.unwrap(I1 << I1 + I1 & ~I1 | I1 << I2 * I3 - I1 & ~I3) == (1 << 1 + 1 & ~1 | 1 << 2 * 3 - 1 & ~3));
        assert(Int.unwrap(I1 << I1 + I1 & ~I1 | I1 << I2 * I3 - I1 & ~I3) == (((1 << (1 + 1)) & (~1)) | ((1 << ((2 * 3) - 1)) & (~3))));
    }

    function test_arithmetic() public pure {
        assert(Int.unwrap(I1 + I2 ** I3 / I4 - I5 % I6 * I7) == (1 + 2 ** 3 / 4 - 5 % 6 * 7));
        assert(Int.unwrap(I1 + I2 ** I3 / I4 - I5 % I6 * I7) == ((1 + ((2 ** 3) / 4)) - ((5 % 6) * 7)));
    }

    function test_not() public pure {
        assert((!I0 + I1) == I2);
        assert((!I0 * I1) != I2);

        assert((!I0 << I2) == I4);
        assert((!I0 | I3) == I3);
        assert((!~-I1 + I1) == I2);
    }

    function test_all() public pure {
        assert(
            Int.unwrap(I128 + I1 - I10 + I4 & ~I1 ^ ~I1 >> I1 + I1 << I3 ** I2 | -I15 % -I10 * I20 / I2 + I13 & ~I3) ==
            (128 + 1 - 10 + 4 & ~1 ^ ~1 >> 1 + 1 << 3 ** 2 | -15 % -10 * 20 / 2 + 13 & ~3)
        );
        assert(
            Int.unwrap(I128 + I1 - I10 + I4 & ~I1 ^ ~I1 >> I1 + I1 << I3 ** I2 | -I15 % -I10 * I20 / I2 + I13 & ~I3) ==
            (
                (
                    ((((128 + 1) - 10) + 4) & (~1)) ^
                    (((~1) >> (1 + 1)) << (3 ** 2))
                ) |
                ((((((-15) % (-10)) * 20) / 2) + 13) & (~3))
            )
        );
    }
}
// ----
// test_bitwise() ->
// test_bitwise_arithmetic() ->
// test_arithmetic() ->
// test_not() ->
// test_all() ->
