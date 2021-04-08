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
// Warning 6328: (111-129): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 0\nz = 0\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (234-253): CHC: Assertion violation happens here.\nCounterexample:\n\nx = (- 1)\ny = 200\nz = 255\n\nTransaction trace:\nC.constructor()\nC.f()
