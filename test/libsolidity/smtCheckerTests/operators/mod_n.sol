pragma experimental SMTChecker;

contract C
{
	function f(uint x, uint y) public pure {
		require(y > 0);
		uint z = x % y;
		assert(z < y);
	}
}
// ----
// Warning 1218: (126-139): Error trying to invoke SMT solver.
