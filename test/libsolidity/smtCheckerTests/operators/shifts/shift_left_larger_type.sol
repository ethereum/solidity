contract C {
	function f() public pure returns (int8) {
		uint8 x = 254;
		int8 y = 1;
		assert(y << x == 0);
		// Fails because z = 0.
		assert(y << x == 10);
		return y << x;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (138-158): CHC: Assertion violation happens here.\nCounterexample:\n\n = 0\nx = 254\ny = 1\n\nTransaction trace:\nC.constructor()\nC.f()
