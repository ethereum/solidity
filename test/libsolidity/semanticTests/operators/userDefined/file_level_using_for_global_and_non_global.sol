==== Source: s1.sol ====
type Int is int;

using {add as +} for Int global;
using {add as +} for Int;

function add(Int a, Int b) pure returns (Int) {
    return Int.wrap(Int.unwrap(a) + Int.unwrap(b));
}

function test_add() pure returns (Int) {
    return Int.wrap(1) + Int.wrap(2);
}

==== Source: s2.sol ====
import "s1.sol";

contract C2 {
    function test1() pure public returns (Int) {
        return test_add();
    }

    function test2() pure public returns (Int) {
        return Int.wrap(3) + Int.wrap(4);
    }
}
// ----
// test1() -> 3
// test2() -> 7
