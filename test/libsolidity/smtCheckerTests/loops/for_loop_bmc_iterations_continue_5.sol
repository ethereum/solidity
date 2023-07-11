contract C
{
	function f() public pure {
		uint x;
		for (; x < 2; ++x) {
			continue;
		}
		// loop expression is executed after continue
		assert(x == 2);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 4
// ----
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
