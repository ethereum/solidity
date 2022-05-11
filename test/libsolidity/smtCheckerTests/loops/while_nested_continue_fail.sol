contract C
{
	function f(uint x, uint y, bool b, bool c) public pure {
		require(x < 10);
		while (x < 10) {
			if (b) {
				x = 15;
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
				assert(y >= 20);
				x = y;
			}
		}
		assert(x >= 20);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (290-305): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 15\nb = false\nc = false\n\nTransaction trace:\nC.constructor()\nC.f(0, 0, false, false)
// Warning 6328: (329-344): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 15\ny = 0\nb = true\nc = false\n\nTransaction trace:\nC.constructor()\nC.f(0, 0, true, false)
