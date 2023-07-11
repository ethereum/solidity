==== Source: Int.sol ====
type Int is int;

using {add as +} for Int global;

function add(Int, Int) pure returns (Int) {}

==== Source: test.sol ====
import "Int.sol";

using {anotherAdd as +} for Int;

function anotherAdd(Int, Int) pure returns (Int) {}

function test() pure returns (Int) {
    return Int.wrap(0) + Int.wrap(0);
}
// ----
// TypeError 3320: (test.sol:26-36): Operators can only be defined in a global 'using for' directive.
// TypeError 4705: (test.sol:26-36): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
