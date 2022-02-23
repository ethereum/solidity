==== Source: s1.sol ====
function f() {
	ecrecover("1234", 1, "0", abi.decode("", (bytes2)));
}
==== Source: s2.sol ====
contract C {}
// ----
// Warning 6133: (s1.sol:16-67): Statement has no effect.
// Warning 2018: (s1.sol:0-70): Function state mutability can be restricted to pure
