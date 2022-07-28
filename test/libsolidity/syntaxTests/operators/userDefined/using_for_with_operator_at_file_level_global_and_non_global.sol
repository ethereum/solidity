==== Source: s1.sol ====
type Int is int;

using {add as +} for Int global;
using {add as +} for Int;

function add(Int a, Int b) pure returns (Int) {}
function test_add() pure returns (Int) {}

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
// TypeError 3320: (s1.sol:58-61): Operators can only be defined in a global 'using for' directive.
