contract C {
	function f() public pure {
		assert(~1 | (~0xff & 0) == -2);
		int x = ~1 | (~0xff ^ 0);
		/// Result is negative, assertion fails.
		assert(x > 0);
		int y = ~x | (0xff & 1);
		assert(y > 0);
		assert(y & (0xffffffffffffffffff & 1) == 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (148-161): CHC: Assertion violation happens here.\nCounterexample:\n\nx = (- 2)\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f()
