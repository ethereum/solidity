pragma experimental SMTChecker;

contract C
{
	function f(uint[] storage a, uint[] storage b, uint[] memory c) internal {
		uint[] memory d = c;
		require(c[0] == 42);
		require(a[0] == 2);
		b[0] = 1;
		// Erasing knowledge about storage references should not
		// erase knowledge about memory references.
		assert(c[0] == 42);
		// Erasing knowledge about storage references should not
		// erase knowledge about memory references.
		assert(d[0] == 42);
		// Fails because b == a is possible.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (497-514): Assertion violation happens here
