type Int is int64;
using {
    bitor as |, bitand as &, bitxor as ^, bitnot as ~,
    add as +, sub as -, unsub as -, mul as *, div as /, mod as %
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

contract C {
    Int constant I0 = Int.wrap(0);
    Int constant I1 = Int.wrap(1);
    Int constant I2 = Int.wrap(2);
    Int constant I3 = Int.wrap(3);
    Int constant I4 = Int.wrap(4);
    Int constant I5 = Int.wrap(5);
    Int constant I6 = Int.wrap(6);
    Int constant I7 = Int.wrap(7);
    Int constant I8 = Int.wrap(8);
    Int constant I10 = Int.wrap(10);
    Int constant I13 = Int.wrap(13);
    Int constant I15 = Int.wrap(15);
    Int constant I20 = Int.wrap(20);
    Int constant I128 = Int.wrap(128);

    function testBitwise() public pure {
        assert(Int.unwrap(I0 & I0 | I1) == (0 & 0 | 1));
        assert(Int.unwrap(I0 & I0 | I1) == ((0 & 0) | 1));
    }

    function testBitwise_arithmetic() public pure {
        assert(Int.unwrap(I2 + I2 & ~I1 | I6 * I6 - I4 & ~I3) == (2 + 2 & ~1 | 6 * 6 - 4 & ~3));
        assert(Int.unwrap(I2 + I2 & ~I1 | I6 * I6 - I4 & ~I3) == (((2 + 2) & (~1)) | (((6 * 6) - 4) & (~3))));
    }

    function testArithmetic() public pure {
        assert(Int.unwrap(I1 + I8 / I4 - I5 % I6 * I7) == (1 + 8 / 4 - 5 % 6 * 7));
        assert(Int.unwrap(I1 + I8 / I4 - I5 % I6 * I7) == ((1 + (8 / 4)) - ((5 % 6) * 7)));
    }

    function testAll() public pure {
        assert(
            Int.unwrap(I128 + I1 - I10 + I4 & ~I1 ^ ~I1 * I2 | -I15 % -I10 * I20 / I2 + I13 & ~I3) ==
            (128 + 1 - 10 + 4 & ~1 ^ ~1 * 2 | -15 % -10 * 20 / 2 + 13 & ~3)
        );
        assert(
            Int.unwrap(I128 + I1 - I10 + I4 & ~I1 ^ ~I1 * I2 | -I15 % -I10 * I20 / I2 + I13 & ~I3) ==
            (
                (
                    ((((128 + 1) - 10) + 4) & (~1)) ^
                    ((~1) * 2)
                ) |
                ((((((-15) % (-10)) * 20) / 2) + 13) & (~3))
            )
        );
    }
}
// ----
// testBitwise() ->
// testBitwise_arithmetic() ->
// testArithmetic() ->
// testAll() ->
