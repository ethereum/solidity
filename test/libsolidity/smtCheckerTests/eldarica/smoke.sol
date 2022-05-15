contract C {
	function f(uint x) public pure {
		assert(x > 0);
	}
}
// ====
// SMTEngine: chc
// SMTSolvers: eld
