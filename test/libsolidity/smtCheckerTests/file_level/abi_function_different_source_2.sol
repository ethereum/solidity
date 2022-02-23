==== Source: s1.sol ====
function f() {
	ecrecover("", 1, "", "");
}
==== Source: s2.sol ====
contract C {}
// ----
// Warning 6133: (s1.sol:16-40): Statement has no effect.
// Warning 2018: (s1.sol:0-43): Function state mutability can be restricted to pure
