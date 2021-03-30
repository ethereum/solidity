pragma experimental SMTChecker;

contract C
{
	uint[][] array2d;
	function p() public {
		array2d.push().push();
	}
	function g(uint x, uint y, uint[] memory c) public {
		require(x < array2d.length);
		require(y < array2d.length);
		f(array2d[x], array2d[y], c);
	}

	function f(uint[] storage a, uint[] storage b, uint[] memory c) internal {
		require(a.length > 0);
		require(b.length > 0);
		require(c.length > 0);
		c[0] = 42;
		a[0] = 2;
		// Access is safe but oob is reported because of aliasing.
		b[0] = 1;
		// Erasing knowledge about storage references should not
		// erase knowledge about memory references.
		assert(c[0] == 42);
		// Fails because b == a is possible.
		assert(a[0] == 2);
		// Access is safe but oob is reported because of aliasing.
		assert(b[0] == 1);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6368: (507-511): CHC: Out of bounds access happens here.
// Warning 6368: (692-696): CHC: Out of bounds access happens here.
// Warning 6328: (685-702): CHC: Assertion violation happens here.
// Warning 6368: (774-778): CHC: Out of bounds access happens here.
