contract C {
	function f() public pure {
		int8 x = 1;
		int8 y = 0;
		assert(x & y != 0);
		x = -1; y = 3;
		assert(x & y == 3);
		y = -1;
		int8 z = x & y;
		assert(z == -1);
		y = 127;
		assert(x & y == 127);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (71-89): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\ny = 0\nz = 0\n\nTransaction trace:\nC.constructor()\nC.f()
