contract C
{
	function f(uint x, uint y, bool b, bool c) public pure {
		require(x < 10);
		while (x < 10) {
			if (b) {
				++x;
				if (x == 10)
					x = 15;
			}
			else {
				require(y < 10);
				while (y < 10) {
					if (c)
						++y;
					else {
						y = 20;
						break;
					}
				}
				assert(y >= 10);
				x = 15;
				break;
			}
		}
		assert(x >= 15);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
