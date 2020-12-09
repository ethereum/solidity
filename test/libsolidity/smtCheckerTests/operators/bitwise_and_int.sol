pragma experimental SMTChecker;

contract C {
	function f() public pure {
		int8 x = 1;
		int8 y = 0;
		assert(x & y != 0);
		x = -1; y = 3;
		assert(x & y == 3);
		y = -1;
		int8 z = x & y;
		assert(z == -1);
		y = 127;
		assert(x & y == 127);
	}
}
// ----
// Warning 6328: (104-122): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
