contract C
{
	function f(uint x, uint y, bool b, bool c) public pure {
		require(x < 10);
		while (x < 10) {
			if (b) {
				x = 20;
				continue;
			}
			else {
				require(y < 10);
				while (y < 10) {
					if (c) {
						y = 20;
						continue;
					}
					y = 15;
					break;
				}
				assert(y >= 15);
				x = y;
			}
		}
		assert(x >= 15);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
