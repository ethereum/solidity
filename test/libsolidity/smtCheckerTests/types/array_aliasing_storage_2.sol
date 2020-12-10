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
// Warning 6328: (436-453): CHC: Assertion violation happens here.\nCounterexample:\narray2d = []\nx = 0\ny = 0\nc = [38, 8, 8, 8, 8, 8, 8, 8, 8]\n\n\nTransaction trace:\nconstructor()\nState: array2d = []\ng(0, 0, [38, 8, 8, 8, 8, 8, 8, 8, 8])
