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
// ====
// SMTEngine: all
// ----
// Warning 6328: (95-109): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 3\nx = 3\n\nTransaction trace:\nC.constructor()\nC.f(3)
