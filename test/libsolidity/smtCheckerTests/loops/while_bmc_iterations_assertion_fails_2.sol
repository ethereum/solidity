contract C {
	function f() public pure {
		uint x;
		while (x < 2) {
			++x;
			break;
		}
		assert(x == 0);
		assert(x == 1);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 2
// ----
// Warning 4661: (93-107): BMC: Assertion violation happens here.
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
