contract C
{
	function f(uint8 x) public pure {
		uint16 y = uint16(x);
		// True because of x's type
		assert(y < 300);
	}
}
// ====
// SMTEngine: all
// ----
