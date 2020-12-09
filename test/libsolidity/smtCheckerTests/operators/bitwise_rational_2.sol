pragma experimental SMTChecker;

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
// ----
// Warning 6328: (181-194): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
