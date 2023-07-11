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
// Warning 6328: (109-123): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
