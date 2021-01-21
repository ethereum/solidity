pragma experimental SMTChecker;

contract C
{
	modifier m(uint a, uint b) {
		require(a > b);
		_;
	}

	function f(uint x) m(x, 0) public pure {
		assert(x > 0);
		assert(x > 1);
	}
}
// ----
// Warning 6328: (164-177): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\na = 1\nb = 0\n\nTransaction trace:\nC.constructor()\nC.f(1)
