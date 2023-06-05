contract C
{
	function f() public pure {
		uint x;
		do {
			++x;
			{
				++x;
				++x;
			}
		} while (x < 3);
		assert(x == 3);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 3
// ----
// Info 6002: BMC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
