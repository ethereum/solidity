contract C
{
	function f() public pure {
		uint x = 0;
		for (uint i = 0; i < 2; ++i) {
			++x;
		}
		// nothing is reported because loop condition is still true after BMCLoopIterations
		assert(x == 3);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 1
// ----
// Info 6002: BMC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
