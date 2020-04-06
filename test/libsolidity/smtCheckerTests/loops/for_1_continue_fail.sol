pragma experimental SMTChecker;

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
// SMTSolvers: z3
// ----
// Warning: (66-72): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (142-156): Assertion violation happens here
