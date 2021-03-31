contract C {
	function f() public pure {
		int8 x = 1;
		int8 y = 0;
		x ^= y;
		assert(x != 1); // fails
		x = -1; y = 1;
		x ^= y;
		assert(x == -2);
		x = 4;
		y ^= x;
		assert(y == 5);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (81-95): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f()
