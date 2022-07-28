type SmallInt is int;
type BigInt is int;

using {add1 as +} for SmallInt;
using {add2 as +} for BigInt;

function add1(SmallInt, SmallInt) pure returns (SmallInt) {
    return SmallInt.wrap(1);
}

function add2(BigInt, BigInt) pure returns (BigInt) {
    return BigInt.wrap(2);
}

contract C {
    function f() public pure returns (SmallInt) {
        return SmallInt.wrap(0) + SmallInt.wrap(0);
    }

    function g() public pure returns (BigInt) {
        return BigInt.wrap(0) + BigInt.wrap(0);
    }
}

// ====
// compileViaYul: also
// ----
// f() -> 1
// g() -> 2
