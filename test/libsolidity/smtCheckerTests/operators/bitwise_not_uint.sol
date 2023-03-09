contract C {
	function f() public pure {
		uint8 x = 0xff;
		assert(~x == 0x00);
		uint16 y = 0xff00;
		assert(~y == 0xff);
		assert(~y == 0xffff);
		assert(~y == 0x0000);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (126-146): CHC: Assertion violation happens here.
// Warning 6328: (150-170): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
