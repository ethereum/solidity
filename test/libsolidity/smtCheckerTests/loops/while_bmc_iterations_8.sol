contract C
{
	uint256[] y;

	function f() public view {
 		uint256 x = 0;
		while (x < y.length) {
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
