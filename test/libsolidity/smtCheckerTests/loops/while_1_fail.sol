pragma experimental SMTChecker;

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
// SMTSolvers: z3
// ----
// Warning 6328: (139-153): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 14\n\n\nTransaction trace:\nconstructor()\nf(14)
