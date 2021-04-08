contract K {
	function f() public pure {
		(abi.encode, 2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (43-58): Statement has no effect.
