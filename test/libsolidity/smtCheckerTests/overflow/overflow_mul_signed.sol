contract C
{
	function f(int8 x) public pure returns (int8) {
		x = 100;
		int8 y;
		unchecked { y = x * 2; }
		assert(y == -56);
		y = x * 100;
		assert(y == 16);
		return y;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (136-143): CHC: Overflow (resulting value larger than 127) happens here.\nCounterexample:\n\nx = 100\n = 0\ny = (- 56)\n\nTransaction trace:\nC.constructor()\nC.f(0)
