pragma experimental SMTChecker;

contract Simple {
	uint[] a;
	function f(uint n) public {
		uint i;
		while (i < n)
		{
			a[i] = i;
			++i;
		}
		require(n > 1);
		// Assertion is safe but current solver version cannot solve it.
		// Keep test for next solver release.
		assert(a[n-1] > a[n-2]);
	}
}
// ----
// Warning: (273-296): Assertion violation happens here
