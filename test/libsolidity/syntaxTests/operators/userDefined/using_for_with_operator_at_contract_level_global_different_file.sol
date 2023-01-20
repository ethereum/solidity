==== Source: s1.sol ====
type Int is int;

==== Source: s2.sol ====
import "s1.sol";

function bitnot(Int) pure returns (Int) {}

contract C {
    using {bitnot as ~} for Int global;
}
// ----
// SyntaxError 3367: (s2.sol:79-114): "global" can only be used at file level.
