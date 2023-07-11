contract C {
	function f(uint x) public pure {
		require(x == 0);
		uint y;
		while (y < 3) {
			++y;
			if (y == 2)
			    x = 3;
		}
		assert(x == 3);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 2
// ----
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
