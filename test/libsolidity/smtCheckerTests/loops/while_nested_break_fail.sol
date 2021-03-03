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
// Warning 6328: (329-344): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 10\nb = false\nc = true\n\nTransaction trace:\nC.constructor()\nC.f(0, 9, false, true)
// Warning 6328: (380-395): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 15\ny = 20\nb = false\nc = false\n\nTransaction trace:\nC.constructor()\nC.f(0, 0, false, false)
