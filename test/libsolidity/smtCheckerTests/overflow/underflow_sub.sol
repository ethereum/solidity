contract C
{
	function f(uint8 x) public pure returns (uint) {
		x = 0;
		uint8 y;
		unchecked { y = x - 1; }
		assert(y == 255);
		unchecked { y = x - 255; }
		assert(y == 1);
		return y;
	}
}
// ====
// SMTEngine: all
// ----
