==== Source: s0.sol ====
type Int is int;

==== Source: s1.sol ====
import "s0.sol";
using {add1 as +} for Int;
using {unsub1 as -} for Int;

function add1(Int, Int) pure returns (Int) {}
function unsub1(Int) pure returns (Int) {}

==== Source: s2.sol ====
import "s0.sol";
using {add2 as +} for Int;
using {unsub2 as -} for Int;

function add2(Int, Int) pure returns (Int) {}
function unsub2(Int) pure returns (Int) {}

==== Source: s3.sol ====
import "s1.sol";
import "s2.sol";
contract C {
    function f() public {
        Int.wrap(0) + Int.wrap(0);
        -Int.wrap(0);
    }
}
// ----
// TypeError 3320: (s1.sol:24-28): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (s1.sol:51-57): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (s2.sol:24-28): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (s2.sol:51-57): Operators can only be defined in a global 'using for' directive.
// TypeError 2271: (s3.sol:81-106): Built-in binary operator + cannot be applied to types Int and Int. No matching user-defined operator found.
// TypeError 4907: (s3.sol:116-128): Built-in unary operator - cannot be applied to type Int. No matching user-defined operator found.
