contract D {
	function f(uint x) public pure {
		assert(x**2 == 4);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 5188: (56-60): Assertion checker does not yet implement this operator.
// Warning 6328: (49-66): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\nTransaction trace:\nD.constructor()\nD.f(0)
// Warning 5188: (56-60): Assertion checker does not yet implement this operator.
