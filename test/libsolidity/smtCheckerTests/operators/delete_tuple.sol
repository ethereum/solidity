contract A{
	function f() public pure {
		delete ([""][0]);
	}
}
// ====
// SMTEngine: all
// ----
// TypeError 4247: (50-57): Expression has to be an lvalue.
