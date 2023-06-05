contract C {
	function f() public pure {
		uint x;
		for (uint i = 0; i < 2; ++i) {
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
// BMCLoopIterations: 3
// ----
// Warning 5740: (77-80): Unreachable code.
// Warning 4661: (108-122): BMC: Assertion violation happens here.
// Info 6002: BMC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
