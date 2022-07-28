==== Source: s1.sol ====
type Int is int;

==== Source: s2.sol ====
import "s1.sol";

function bitnot(Int) pure returns (Int) {}

using {bitnot as ~} for Int global;
// ----
// TypeError 4117: (s2.sol:62-97): Can only use "global" with types defined in the same source unit at file level.
