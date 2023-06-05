contract C {
	function f() public pure {
		uint x = 0;
		do {
			++x;
			if (x > 1) {
				++x;
				break;
			}
		} while (x < 3);
		assert(x == 3);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 4
// ----
// Info 6002: BMC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
