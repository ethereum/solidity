==== Source: s1.sol ====
type Int is int;
using {add as +} for Int global;
using {sub as -} for Int;

function add(Int, Int) pure returns (Int) {}
function sub(Int, Int) pure returns (Int) {}

==== Source: s2.sol ====
import "s1.sol";
contract C {
    function f() pure public {
        Int.wrap(0) + Int.wrap(0);
        Int.wrap(0) - Int.wrap(0);
    }
}
// ----
// TypeError 2271: (s2.sol:104-129): Built-in binary operator - cannot be applied to types Int and Int. No matching user-defined operator found.
