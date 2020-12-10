pragma experimental SMTChecker;
contract C {
	function f(uint x) public pure {
		require(x == 2);
		uint y = 10;
		y /= y / x;
		assert(y == x);
		assert(y == 0);
	}
}
// ----
// Warning 6328: (147-161): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 2\n\n\nTransaction trace:\nconstructor()\nf(2)
