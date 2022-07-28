==== Source: Int.sol ====
type Int is int;

using {add as +} for Int global;

function add(Int, Int) pure returns (Int) {}

==== Source: test.sol ====
import "Int.sol";

using {another_add as +} for Int;

function another_add(Int, Int) pure returns (Int) {}

function test() pure returns (Int) {
    return Int.wrap(0) + Int.wrap(0);
}
// ----
// TypeError 5583: (test.sol:156-181): User-defined binary operator + has more than one definition matching the operand types visible in the current scope.
