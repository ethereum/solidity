pragma experimental SMTChecker;

contract Simple {
	uint[] a;
	function f(uint n) public {
		uint i;
		for (i = 0; i < n; ++i)
			a[i] = i;
		require(n > 1);
		// Assertion is safe but current solver version times out.
		// Keep test for next solver release.
		assert(a[n-1] > a[n-2]);
	}
}
// ----
// Warning: (261-284): Error trying to invoke SMT solver.
// Warning: (261-284): Assertion violation happens here
