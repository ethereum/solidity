contract C
{
	uint x;

	function condition() private returns(bool) {
		++x;
		return x < 3;
	}

	function f() public {
		require(x == 0);
		for (; condition();) {
		}
		assert(x == 3);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 5
// ----
// Warning 2661: (71-74): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
