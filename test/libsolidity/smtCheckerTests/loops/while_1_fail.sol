contract C
{
	function f(uint x) public pure {
		require(x < 100);
		while (x < 10) {
			x = x + 1;
		}
		assert(x < 14);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 6328: (106-120): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 14\n\nTransaction trace:\nC.constructor()\nC.f(14)
