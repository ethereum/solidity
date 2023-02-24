type Int is int;
using {add as +, mul as *, unsub as -} for Int global;

function add(Int a, Int b) pure returns (Int) {
    return Int.wrap(Int.unwrap(a) + Int.unwrap(b));
}

function mul(Int a, Int b) pure returns (Int) {
    return Int.wrap(Int.unwrap(a) * Int.unwrap(b));
}

function unsub(Int a) pure returns (Int) {
    return Int.wrap(-Int.unwrap(a));
}

function i(int x) pure suffix returns (Int) {
    return Int.wrap(x);
}

contract C {
    function test() public pure returns (Int) {
        return - - -(4 i + 4 i * -3 i) * 2 i;
    }
}
// ----
// test() -> 16
