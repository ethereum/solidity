pragma experimental SMTChecker;

contract C
{
	uint[2] b;
	function f(uint[2] storage a, uint[2] memory c) internal {
		require(c[0] == 42);
		require(a[0] == 2);
		b[0] = 1;
		// Erasing knowledge about storage variables should not
		// erase knowledge about memory references.
		assert(c[0] == 42);
		// Fails because b == a is possible.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (342-359): Assertion violation happens here
