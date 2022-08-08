==== Source: s0.sol ====
type Int is int;

==== Source: s1.sol ====
import "s0.sol";
using {add1 as +} for Int;

function add1(Int, Int) returns (Int) {
    return Int.wrap(3);
}

==== Source: s2.sol ====
import "s0.sol";
using {add2 as +} for Int;

function add2(Int, Int) returns (Int) {
    return Int.wrap(7);
}

==== Source: s3.sol ====
import "s1.sol";
import "s2.sol";
contract C {
    function f() public {
        Int.wrap(0) + Int.wrap(0);
    }
}

// ----
// TypeError 2271: (s3.sol:81-106): Operator + not compatible with types Int and Int. No matching user-defined operator found.
