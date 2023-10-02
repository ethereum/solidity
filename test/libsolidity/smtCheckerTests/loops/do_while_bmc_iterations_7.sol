contract C
{
	uint256[] y;

	function f() public view {
		uint256 x = 0;
		do {
			++x;
		} while (x < y.length);
		require(x != 0);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 1
// ----
// Warning 6838: (124-130): BMC: Condition is always true.
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
