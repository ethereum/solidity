contract C {
	function f() public pure {
		uint x;
		do {
			++x;
			if (x > 1) {
				x = 3;
				break;
 			}
			if (x > 0) {
				x = 2;
				break;
 			}
		} while (x < 3);
		assert(x == 2);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 3
// ----
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
