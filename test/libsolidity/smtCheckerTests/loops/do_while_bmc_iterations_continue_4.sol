contract C
{
	function f(uint x) public pure {
		require(x == 0);
		uint i;
		do {
			++i;
			if (i == 2) {
				x = 2;
				continue;
			}
			if (i == 1) {
				x = 1;
				continue;
			}
		} while (i < 3);
		assert(x == 2);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 4
// ----
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
