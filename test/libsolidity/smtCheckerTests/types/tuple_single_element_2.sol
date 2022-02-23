contract C {
	function f() public pure {
		(("", ""));
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (43-53): Statement has no effect.
