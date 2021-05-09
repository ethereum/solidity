contract C {
	function f() public pure {
		(("", 2));
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (43-52): Statement has no effect.
