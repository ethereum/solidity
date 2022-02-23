contract C {
	function f() public pure {
		int8 x = 1;
		int8 y = 0;
		x &= y;
		assert(x != 0); // fails
		x = -1; y = 3;
		y &= x;
		assert(y == 3);
		y = -1;
		y &= x;
		assert(y == -1);
		y = 127;
		x &= y;
		assert(x == 127);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (81-95): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f()
