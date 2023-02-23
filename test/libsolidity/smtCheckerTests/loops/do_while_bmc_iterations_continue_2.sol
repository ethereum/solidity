contract C {
	function f() public pure {
		uint x = 0;
		uint y = 0;
		do {
			++x;
			if (x == 2) {
				++x;
				y = 1;
				continue;
			}
			if (x == 3) {
				y = 2;
				continue;
			}
		} while (x < 3);
		assert(y == 1);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 4
// ----
// Info 6002: BMC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
