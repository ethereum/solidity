contract C
{
	function f(uint x) public pure {
		require(x == 0);
		do {
			++x;
		} while (x < 2);
		assert(x == 3);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 3
// ----
// Warning 4661: (102-116): BMC: Assertion violation happens here.
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
