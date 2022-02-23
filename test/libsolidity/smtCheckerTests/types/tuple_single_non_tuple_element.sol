contract C {
	function f() public pure {
		(2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (43-46): Statement has no effect.
