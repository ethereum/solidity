contract C {
	function f() public pure {
		uint8 x = 0xff;
		uint8 y = ~x;
		assert(x & y == 0);
		assert(x | y == 0xff);
		assert(x ^ y == 0xff);
	}
}
// ====
// SMTEngine: all
// ----
