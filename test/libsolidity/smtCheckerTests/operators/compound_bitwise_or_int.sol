contract C {
	function f() public pure {
		int8 x = 1;
		int8 y = 0;
		x |= y;
		assert(x == 0); // fails
		x = -1; y = 3;
		x |= y;
		assert(x == -1);
		x = 4;
		y |= x;
		assert(y == 7);
		y = 127;
		x |= y;
		assert(x == 127);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (81-95): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
