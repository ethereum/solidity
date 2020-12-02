pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 100);
		for(uint i = 0; i < 10; ++i) {
			// Overflows due to resetting x.
			x = x + 1;
		}
		assert(x < 14);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning 4984: (176-181): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (189-203): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 14\n\n\nTransaction trace:\nconstructor()\nf(4)
// Warning 2661: (176-181): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
