contract C
{
	function f(uint x, bool b) public pure {
		require(x < 10);
		for (; x < 10; ) {
			++x;
		}
		assert(x > 15);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 5667: (33-39): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (109-123): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 10\nb = false\n\nTransaction trace:\nC.constructor()\nC.f(9, false)
