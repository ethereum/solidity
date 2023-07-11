contract C {
	function f() public pure {
		uint8 x = 1;
		uint16 y = 0;
		assert(x ^ y != 0);
		x = 0xff;
		y = 0xff00;
		assert(x ^ y == 0xff);
		assert(x ^ y == 0xffff);
		assert(x ^ y == 0x0000);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (122-143): CHC: Assertion violation happens here.
// Warning 6328: (174-197): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
