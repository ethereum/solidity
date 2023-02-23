contract C {
	function f() public pure {
		uint x = 0;
		while (x < 5) {
			x = x + 1;
		}
		// nothing is reported because loop condition is true after unrolling the loop one time
		assert(x == 5);
		assert(x == 3);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 1
// ----
// Info 6002: BMC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
