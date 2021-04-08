contract C
{
	modifier m(uint a, uint b) {
		require(g(a, b));
		_;
	}

	modifier notZero(uint x) {
		require(x > 0);
		_;
	}

	function g(uint a, uint b) notZero(a) internal pure returns (bool) {
		return a > b;
	}

	function f(uint x) m(x, 0) public pure {
		assert(x > 0);
		assert(x > 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (278-291): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\na = 1\nb = 0\n\nTransaction trace:\nC.constructor()\nC.f(1)\n    C.g(1, 0) -- internal call
