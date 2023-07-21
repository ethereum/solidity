contract C
{
	uint x;
	uint y;

	function condition() private returns(bool) {
		x = (x + 1) % 2;
		return (x == 1);
	}

	function f() public {
		require(x == 0);
		require(y == 0);
		for (; condition();) {
			++y;
		}
		assert(y == 1);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 5
// ----
// Warning 2661: (85-90): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Info 6002: BMC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
