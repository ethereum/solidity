pragma experimental SMTChecker;
contract C {
	function f(uint x) public pure {
		require(x == 2);
		uint y = 10;
		y /= y / x;
		assert(y == x);
		assert(y == 0);
	}
}
// ----
// Warning: (129-143): Error trying to invoke SMT solver.
// Warning: (147-161): Error trying to invoke SMT solver.
// Warning: (147-161): Assertion violation happens here
