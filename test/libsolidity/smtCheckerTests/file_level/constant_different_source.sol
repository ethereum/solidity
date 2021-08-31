==== Source: l.sol ====
library L {
	int constant one = 1;
	function f() internal {
		one;
	}
}
==== Source: s1.sol ====
library L {}
// ----
// Warning 6133: (l.sol:62-65): Statement has no effect.
// Warning 2018: (l.sol:36-69): Function state mutability can be restricted to pure
