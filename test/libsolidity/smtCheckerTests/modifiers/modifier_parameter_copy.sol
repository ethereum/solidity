pragma experimental SMTChecker;

contract C
{
	modifier m(uint x) {
		x == 2;
		_;
	}

	function f(uint x) m(x) public pure {
		assert(x == 2);
	}
}
// ----
// Warning 6328: (128-142): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\n\nTransaction trace:\nconstructor()\nf(0)
