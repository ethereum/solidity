contract C {
	function f() public pure {
		uint x = 0;
		int y = 0;
		do {
			if (x >= 3)
				y = 1;
			++x;
		} while (x < 3 || y == 1);
		// BMC loop iteration setting is more than enough to leave the loop
		assert(x == 3);
		assert(y == 1); // should fail
		assert(y == 0);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 5
// ----
// Warning 4661: (228-242): BMC: Assertion violation happens here.
// Info 6002: BMC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
