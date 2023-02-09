contract C {
	function f() public pure {
		uint x;
		do {
			continue;
			x = 1;
		} while (x == 0);
		assert(x == 0);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 5740: (74-79): Unreachable code.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
