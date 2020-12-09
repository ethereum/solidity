pragma experimental SMTChecker;

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
// ----
// Warning 6328: (171-191): CHC: Assertion violation happens here.\nCounterexample:\n\n\n = 0\n\nTransaction trace:\nconstructor()\nf()
