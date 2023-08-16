contract C {
	function f() public pure {
		uint x;
		do {
			++x;
			break;
		} while (x < 3);
		assert(x == 1);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 4
// ----
// Warning 5740: (87-92): Unreachable code.
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
