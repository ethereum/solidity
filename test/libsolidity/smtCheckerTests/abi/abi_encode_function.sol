pragma experimental SMTChecker;
contract C {
	function f() public view {
		abi.encode(this.f);
	}
}
// ----
// Warning 6031: (86-92): Internal error: Expression undefined for SMT solver.
// Warning 6031: (86-92): Internal error: Expression undefined for SMT solver.
