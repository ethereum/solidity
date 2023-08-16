contract C {
	function f() public pure {
		uint x = 0;
		int y = 0;
		for (; x < 3 || y == 1; ++x) {
			if (x >= 3)
				y = 1;
		}
        // BMC loop iteration setting is not enough to leave the loop
		assert(x == 0); // should hold - no assumptions on value if didn't complete the loop
		assert(y == 0); // should hold - no assumptions on value if didn't complete the loop
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 2
// ----
// Info 6002: BMC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
