contract C
{
	modifier m {
		uint x = 2;
		_;
	}

	function f(uint x) m public pure {
		assert(x == 2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (88-102): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 3\nx = 2\n\nTransaction trace:\nC.constructor()\nC.f(3)
