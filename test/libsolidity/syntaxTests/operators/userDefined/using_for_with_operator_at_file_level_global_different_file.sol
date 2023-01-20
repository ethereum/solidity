==== Source: s1.sol ====
type Bool is bool;

==== Source: s2.sol ====
import "s1.sol";

function not(Bool) pure returns (bool) {}

using {not as !} for Bool global;
// ----
// TypeError 4117: (s2.sol:61-94): Can only use "global" with types defined in the same source unit at file level.
