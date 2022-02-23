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
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 4984: (143-148): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (156-170): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 14\n\nTransaction trace:\nC.constructor()\nC.f(4)
// Warning 2661: (143-148): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
