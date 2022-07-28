type Uint is uint;
using {unaryCountdown as ~, binaryCountdown as ^, eq as ==} for Uint global;

function unaryCountdown(Uint x) pure returns (Uint) {
    if (x == Uint.wrap(0))
        return Uint.wrap(0);

    return ~Uint.wrap(Uint.unwrap(x) - 1);
}

function binaryCountdown(Uint x, Uint y) pure returns (Uint) {
    if (x == Uint.wrap(0) && y == Uint.wrap(0))
        return Uint.wrap(0);
    if (x == Uint.wrap(0))
        return y ^ x;

    return Uint.wrap(Uint.unwrap(x) - 1) ^ y;
}

function eq(Uint x, Uint y) pure returns (bool) {
    return Uint.unwrap(x) == Uint.unwrap(y);
}

contract C {
    function testUnary(Uint x) public pure returns (Uint) {
        return ~x;
    }

    function testBinary(Uint x, Uint y) public pure returns (Uint) {
        return x ^ y;
    }
}
// ----
// testUnary(uint256): 0 -> 0
// testUnary(uint256): 1 -> 0
// testUnary(uint256): 99999999999 -> FAILURE
// testBinary(uint256,uint256): 0, 0 -> 0
// testBinary(uint256,uint256): 1, 0 -> 0
// testBinary(uint256,uint256): 0, 1 -> 0
// testBinary(uint256,uint256): 1, 1 -> 0
// testBinary(uint256,uint256): 99999999999, 99999999999 -> FAILURE
