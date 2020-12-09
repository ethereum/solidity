pragma experimental SMTChecker;

contract C
{
	function f(uint x, bool b) public pure {
		require(x < 100);
		while (x < 10) {
			if (b) {
				x = 15;
				continue;
				x = 200;
			}
			x = 20;

		}
		// Fails due to the if.
		assert(x >= 17);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning 5740: (169-176): Unreachable code.
// Warning 6328: (227-242): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 10\nb = false\n\n\nTransaction trace:\nconstructor()\nf(10, false)
