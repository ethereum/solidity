contract Simple {
	function f(uint x) public pure {
		uint y;
		for (y = 0; y < x; ++y) {}
		assert(y == x);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
