pragma experimental SMTChecker;

contract C
{
	uint[] b;
	uint[] d;
	uint[][] array2d;
	function g(uint x, uint[] memory c) public {
		f(array2d[x], c);
	}
	function f(uint[] storage a, uint[] memory c) internal {
		d[0] = 42;
		c[0] = 42;
		a[0] = 2;
		b[0] = 1;
		// Erasing knowledge about storage variables should not
		// erase knowledge about memory references.
		assert(c[0] == 42);
		// Fails because d == a is possible.
		// Removed because current Spacer seg faults in cex generation.
		//assert(d[0] == 42);
		// Fails because b == a and d == a are possible.
		assert(a[0] == 2);
		// b == a is possible, but does not fail because b
		// was the last assignment.
		assert(b[0] == 1);
	}
}
// ----
// Warning 6328: (572-589): CHC: Assertion violation happens here.\nCounterexample:\nb = [1, 20, 20, 20, 20], d = [], array2d = []\nx = 0\nc = [0, 9, 9, 9, 9, 9, 9, 9, 9, 9]\n\n\nTransaction trace:\nconstructor()\nState: b = [], d = [], array2d = []\ng(0, [0, 9, 9, 9, 9, 9, 9, 9, 9, 9])
