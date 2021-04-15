contract C {
	function f() public view {
		abi.encode(this.f);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6031: (54-60): Internal error: Expression undefined for SMT solver.
// Warning 6031: (54-60): Internal error: Expression undefined for SMT solver.
