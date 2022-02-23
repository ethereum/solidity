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
// ====
// SMTEngine: all
// ----
// Warning 6328: (131-144): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\na = 1\nb = 0\n\nTransaction trace:\nC.constructor()\nC.f(1)
