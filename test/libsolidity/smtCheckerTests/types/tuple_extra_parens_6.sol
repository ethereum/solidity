contract C {
	function f() public pure {
		(((,))) = ((2),3);
	}
}
// ====
// SMTEngine: all
// ----
