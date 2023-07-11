type SmallInt is int;
type BigInt is int;

using {addSmall as +} for SmallInt global;
using {addBig as +} for BigInt global;

function addSmall(SmallInt a, SmallInt b) pure returns (SmallInt) {
    return SmallInt.wrap(SmallInt.unwrap(a) + SmallInt.unwrap(b));
}

function addBig(BigInt a, BigInt b) pure returns (BigInt) {
    return BigInt.wrap(10 * (BigInt.unwrap(a) + BigInt.unwrap(b)));
}

contract C {
    function small() public pure returns (SmallInt) {
        return SmallInt.wrap(1) + SmallInt.wrap(2);
    }

    function big() public pure returns (BigInt) {
        return BigInt.wrap(3) + BigInt.wrap(4);
    }
}
// ----
// small() -> 3
// big() -> 70
