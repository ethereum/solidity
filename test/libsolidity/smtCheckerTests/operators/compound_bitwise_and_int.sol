pragma experimental SMTChecker;

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
// ----
// Warning 6328: (114-128): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
