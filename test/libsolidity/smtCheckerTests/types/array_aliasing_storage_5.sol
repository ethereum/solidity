pragma experimental SMTChecker;

contract C
{
	uint[] b;
	uint[] d;
	function f(uint[] storage a, uint[] memory c) internal {
		require(d[0] == 42);
		require(c[0] == 42);
		require(a[0] == 2);
		b[0] = 1;
		// Erasing knowledge about storage variables should not
		// erase knowledge about memory references.
		assert(c[0] == 42);
		// Should not fail since b == d is not possible.
		assert(d[0] == 42);
		// Fails because b == a is possible.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (446-463): Assertion violation happens here
