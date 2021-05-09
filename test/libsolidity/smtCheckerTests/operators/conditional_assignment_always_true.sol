contract C {
	function f(bool b) public pure {
		require(b);
		uint c = b ? 5 : 1;
		assert(c < 5);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (85-98): CHC: Assertion violation happens here.\nCounterexample:\n\nb = true\nc = 5\n\nTransaction trace:\nC.constructor()\nC.f(true)
// Warning 6838: (72-73): BMC: Condition is always true.
