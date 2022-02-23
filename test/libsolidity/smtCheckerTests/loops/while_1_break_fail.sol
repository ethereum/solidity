contract C
{
	function f(uint x, bool b) public pure {
		require(x < 10);
		while (x < 10) {
			if (b)
				++x;
			else {
				break;
			}
		}
		// Fails because the loop might break.
		assert(x >= 10);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 6328: (185-200): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\nb = false\n\nTransaction trace:\nC.constructor()\nC.f(0, false)
