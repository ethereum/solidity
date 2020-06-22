pragma experimental SMTChecker;

contract C
{
	function f(uint16 x, uint16 y) public pure {
		require(y > 0);
		uint z = x % y;
		assert(z < 100_000);
	}
}
// ----
// Warning 1218: (130-149): Error trying to invoke SMT solver.
