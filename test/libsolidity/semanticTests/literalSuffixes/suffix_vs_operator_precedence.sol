type Int is int;
using {sub as -, unsub as -} for Int global;

function sub(Int a, Int b) pure returns (Int) {
    return Int.wrap((Int.unwrap(a) - Int.unwrap(b)) * 100);
}

function unsub(Int a) pure returns (Int) {
    return Int.wrap(Int.unwrap(a) + 10);
}

function u(int x) pure suffix returns (Int) {
    return Int.wrap(x + 1);
}

contract C {
    function testUnary() public pure returns (Int) {
        return -2 u;
    }

    function testUnaryWithParens() public pure returns (Int) {
        return -(2 u);
    }

    function testBinary() public pure returns (Int) {
        return 4 u - 2 u;
    }
}
// ----
// testUnary() -> 13
// testUnaryWithParens() -> 13
// testBinary() -> 200
