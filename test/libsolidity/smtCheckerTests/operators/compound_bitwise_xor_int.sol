pragma experimental SMTChecker;

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
// ----
// Warning 6328: (114-128): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
