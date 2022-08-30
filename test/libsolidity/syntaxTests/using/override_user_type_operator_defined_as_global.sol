==== Source: s1.sol ====
type Int is int;
using {add as +} for Int global;

function add(Int, Int) pure returns (Int) {
    return Int.wrap(3);
}

==== Source: s2.sol ====
import "s1.sol";

using {another_add as +} for Int;

function another_add(Int, Int) pure returns (Int) {
    return Int.wrap(3);
}

contract C {
    function f() pure public {
        Int.wrap(0) + Int.wrap(0);
    }
}
// ----
// TypeError 2271: (s2.sol:184-209): Binary operator + not compatible with types Int and Int. Multiple user-defined functions provided for this operator.
