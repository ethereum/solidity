pragma experimental SMTChecker;

contract C
{
	function f(uint8 x) public pure returns (uint8) {
		uint8 y;
		unchecked { y = x + 255; }
		require(y >= x);
		x = 255;
		unchecked { y = x + 1; }
		assert(y == 0);
		y = x + 255;
		assert(y == 254);
		return y;
	}
}
// ----
// Warning 4984: (218-225): CHC: Overflow (resulting value larger than 255) happens here.\nCounterexample:\n\nx = 255\n = 0\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f(0)
