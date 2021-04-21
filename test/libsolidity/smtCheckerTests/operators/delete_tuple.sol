contract A{
	function f() public pure {
		delete ([""][0]);
	}
}
// ====
// SMTEngine: all
// ----
