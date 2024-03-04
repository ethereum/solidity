contract C
{
	function f(uint x) public pure {
		require(x == 0);
		for (uint i = 0; i < 2; ++i) {
			for (uint j = 0; j < 2; ++j) {
				x = x + 1;
				break;
			}
			break;
		}
		assert(x == 1);
	}
}
// ====
// SMTEngine: bmc
// SMTSolvers: z3
// BMCLoopIterations: 3
// ----
// Warning 5740: (92-95): Unreachable code.
// Warning 5740: (126-129): Unreachable code.
// Info 6002: BMC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
