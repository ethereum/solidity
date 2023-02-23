contract C
{
	function f() public pure {
		uint x;
		do {
			if (x >= 2)
				++x;
			++x;
		} while (x < 3);
		assert(x == 4);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 5
// ----
// Info 6002: BMC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
