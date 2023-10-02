contract C {
	function f() public pure {
		int8 x = 1;
		int8 y = 0;
		assert(x & y != 0); // should fail
		x = -1; y = 3;
		assert(x & y == 3); // should hold
		y = -1;
		int8 z = x & y;
		assert(z == -1); // should hold
		y = 127;
		assert(x & y == 127); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 6328: (71-89): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\ny = 0\nz = 0\n\nTransaction trace:\nC.constructor()\nC.f()
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
