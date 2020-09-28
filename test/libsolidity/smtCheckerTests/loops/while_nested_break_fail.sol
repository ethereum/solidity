pragma experimental SMTChecker;

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
				assert(y >= 15);
				x = 15;
				break;
			}
		}
		assert(x >= 20);
	}
}
// ----
// Warning 6328: (329-344): CHC: Assertion violation happens here.
// Warning 6328: (380-395): CHC: Assertion violation happens here.
