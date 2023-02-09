contract C {
	function f() public pure {
		int16 x = 1;
		int16 y = 0;
		assert(x | y == 1);
		x = 0; y = 0;
		assert(x | y != 0);
		y = 240;
		x = 15;
		int16 z = x | y;
		assert(z == 255);
		x = -1; y = 200;
		assert(x | y == x);
		assert(x | z != -1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (111-129): CHC: Assertion violation happens here.
// Warning 6328: (234-253): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
