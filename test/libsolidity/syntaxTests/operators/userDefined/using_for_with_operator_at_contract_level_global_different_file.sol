==== Source: s1.sol ====
type Bool is bool;

==== Source: s2.sol ====
import "s1.sol";

function not(Bool) pure returns (Bool) {}

contract C {
    using {not as !} for Bool global;
}
// ----
// SyntaxError 3367: (s2.sol:78-111): "global" can only be used at file level.
