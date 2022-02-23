contract C
{
	function f(int8 x) public pure returns (int8) {
		x = -2;
		int8 y;
		unchecked { y = x - 127; }
		assert(y == 127);
		x = -128;
		unchecked { y = x - 127; }
		assert(y == 1);
		x = 127;
		unchecked { y = x - (-127); }
		assert(y == -2);
		return y;
	}
}
// ====
// SMTEngine: all
// ----
