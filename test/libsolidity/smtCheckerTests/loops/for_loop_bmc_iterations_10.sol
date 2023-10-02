contract C
{
	uint256[] y;

	function f() public view {
		uint256 x = 0;
		for (uint i = 0; i < y.length; i++) {
			x = 1;
		}
		// tests that constant condition warning is not reported
		require(x != 0);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 1
// ----
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
