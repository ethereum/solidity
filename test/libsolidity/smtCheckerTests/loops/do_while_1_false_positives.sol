pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 100);
		do {
			x = x + 1;
		} while (x < 1000);
		assert(x > 0);
	}
}
// ====
// SMTSolvers: z3
