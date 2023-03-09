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
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
