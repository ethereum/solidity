pragma experimental SMTChecker;

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
// SMTSolvers: z3
// ----
// Warning 6328: (218-233): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\nb = false\n\n\nTransaction trace:\nconstructor()\nf(0, false)
