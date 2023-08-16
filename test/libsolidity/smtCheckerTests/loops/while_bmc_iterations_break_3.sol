contract C {
	function f() public pure {
		uint x;
		while (x < 3) {
			if (x > 0) {
				++x;
				break;
			}
			++x;
		}
		assert(x == 2);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 3
// ----
// Info 6002: BMC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
