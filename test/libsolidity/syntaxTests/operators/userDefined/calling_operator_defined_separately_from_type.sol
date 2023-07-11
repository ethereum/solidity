==== Source: definition.sol ====
import "type-and-binding.sol";

function add(Int, Int) pure returns (Int) {}
function unsub(Int) pure returns (Int) {}

==== Source: type-and-binding.sol ====
import "definition.sol";

type Int is int;

using {add as +} for Int global;
using {unsub as -} for Int global;

==== Source: use.sol ====
import "type-and-binding.sol";

contract C {
    function f() pure public {
        Int.wrap(0) + Int.wrap(0);
        -Int.wrap(0);
    }
}
