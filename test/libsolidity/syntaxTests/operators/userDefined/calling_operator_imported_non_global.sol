==== Source: s1.sol ====
type Int is int;
using {add as +} for Int;
using {unsub as -} for Int;

function add(Int, Int) pure returns (Int) {}
function unsub(Int) pure returns (Int) {}

==== Source: s2.sol ====
import "s1.sol";

contract C {
    function f() pure public {
        Int.wrap(0) + Int.wrap(0);
        -Int.wrap(0);
    }
}
// ----
// TypeError 3320: (s1.sol:24-27): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (s1.sol:50-55): Operators can only be defined in a global 'using for' directive.
// TypeError 2271: (s2.sol:70-95): Built-in binary operator + cannot be applied to types Int and Int. No matching user-defined operator found.
// TypeError 4907: (s2.sol:105-117): Built-in unary operator - cannot be applied to type Int. No matching user-defined operator found.
