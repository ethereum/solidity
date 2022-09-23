==== Source: Int.sol ====
type Int is int;

using {add as +} for Int global;

function add(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

==== Source: test.sol ====
import "Int.sol";

using {another_add as +} for Int;

function another_add(Int, Int) pure returns (Int) {
    return Int.wrap(1);
}

function test() pure returns (Int) {
    return Int.wrap(0) + Int.wrap(0);
}

// ----
// TypeError 2271: (test.sol:181-206): Built-in binary operator + cannot be applied to types Int and Int. Multiple user-defined functions provided for this operator.
