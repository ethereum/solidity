contract C {
	function f() public pure {
		uint x;
		while (true) {
			++x;
		}
		assert(x == 1000);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 1
// ----
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
