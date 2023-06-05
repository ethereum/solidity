contract C
{
	function f() public pure {
		uint x = 0;
		for (;;) {
			x = 1;
		}
		assert(x == 1000);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 3
// ----
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
