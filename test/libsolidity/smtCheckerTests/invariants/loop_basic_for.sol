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
