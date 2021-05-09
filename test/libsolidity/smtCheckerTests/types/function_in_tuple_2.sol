contract K {
	function f() public pure {
		(abi.encode, "");
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (43-59): Statement has no effect.
