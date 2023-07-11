type Int is int16;

using {keccak256 as +} for Int global;

function keccak256(Int a, Int b) pure returns (Int) {
    return Int.wrap(Int.unwrap(a) + Int.unwrap(b));
}

contract C {
    function test() public returns (Int) {
        return Int.wrap(3) + Int.wrap(4);
    }
}
// ----
// test() -> 7
