pragma experimental SMTChecker;

contract C
{
	uint[][] array2d;
	function g(uint x, uint y, uint[] memory c) public {
		f(array2d[x], array2d[y], c);
	}

	function f(uint[] storage a, uint[] storage b, uint[] memory c) internal {
		c[0] = 42;
		a[0] = 2;
		b[0] = 1;
		// Erasing knowledge about storage references should not
		// erase knowledge about memory references.
		assert(c[0] == 42);
		// Fails because b == a is possible.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (436-453): Assertion violation happens here
// Warning: (436-453): Assertion violation happens here
