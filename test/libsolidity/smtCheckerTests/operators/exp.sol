pragma experimental SMTChecker;
contract D {
	function f(uint x) public pure {
		assert(x**2 == 4);
	}
}
// ----
// Warning 5188: (88-92): Assertion checker does not yet implement this operator.
// Warning 6328: (81-98): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\n\nTransaction trace:\nconstructor()\nf(0)
// Warning 5188: (88-92): Assertion checker does not yet implement this operator.
