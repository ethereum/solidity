abstract contract A {
	function f() public pure {
		assert(false); // A cannot be deployed so this should not be reported
	}
}
// ====
// SMTEngine: all
// ----
