contract C
{
	function f(uint x, bool b) public pure {
		require(x < 10);
		while (x < 10) {
			if (b) {
				x = 20;
				continue;
			}
			++x;
		}
		assert(x >= 10);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
