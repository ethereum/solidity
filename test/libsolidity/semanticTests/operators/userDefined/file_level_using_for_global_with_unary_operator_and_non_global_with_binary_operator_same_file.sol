type Int is int16;

using {unsub as -} for Int global;
using {sub as -} for Int;

function sub(Int a, Int b) pure returns (Int) {
    return Int.wrap(Int.unwrap(a) - Int.unwrap(b));
}

function unsub(Int a) pure returns (Int) {
    return Int.wrap(-Int.unwrap(a));
}

contract C {
    function test_sub() public returns (Int) {
        return Int.wrap(7) - Int.wrap(2);
    }

    function test_unsub() public returns (Int) {
        return -Int.wrap(4);
    }
}
// ----
// test_sub() -> 5
// test_unsub() -> -4
