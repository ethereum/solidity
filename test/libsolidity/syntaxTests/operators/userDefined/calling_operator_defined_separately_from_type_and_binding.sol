==== Source: binding.sol ====
import "definition.sol";
import "type.sol";

using {add as +} for Int global;
using {unsub as -} for Int global;

==== Source: definition.sol ====
import "type.sol";

function add(Int, Int) pure returns (Int) {}
function unsub(Int) pure returns (Int) {}

==== Source: type.sol ====
type Int is int;

==== Source: use.sol ====
import "type.sol";

contract C {
    function f() pure public {
        Int.wrap(0) + Int.wrap(0);
        -Int.wrap(0);
    }
}
// ----
// TypeError 4117: (binding.sol:45-77): Can only use "global" with types defined in the same source unit at file level.
// TypeError 4117: (binding.sol:78-112): Can only use "global" with types defined in the same source unit at file level.
// TypeError 2271: (use.sol:72-97): Built-in binary operator + cannot be applied to types Int and Int. No matching user-defined operator found.
// TypeError 4907: (use.sol:107-119): Built-in unary operator - cannot be applied to type Int. No matching user-defined operator found.
