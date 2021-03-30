pragma experimental SMTChecker;

contract C
{
	uint[][] array2d;
	function p() public { array2d.push().push(); }
	function g(uint x, uint y, uint[] memory c) public {
		require(x < array2d.length);
		require(y < array2d.length);
		f(array2d[x], array2d[y], c);
	}
	function f(uint[] storage a, uint[] storage b, uint[] memory c) internal {
		require(a.length > 0);
		require(b.length > 0);
		require(c.length > 0);
		uint[] memory d = c;
		c[0] = 42;
		a[0] = 2;
		// Access is safe but oob is reported due of aliasing.
		b[0] = 1;
		// Erasing knowledge about storage references should not
		// erase knowledge about memory references.
		// Disabled because of Spacer's seg fault.
		//assert(c[0] == 42);
		// Erasing knowledge about storage references should not
		// erase knowledge about memory references.
		// Disabled because of Spacer's seg fault.
		//assert(d[0] == 42);
		// Fails because b == a is possible.
		// Accesses are safe but oob is reported due of aliasing.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 2072: (417-432): Unused local variable.
// Warning 6368: (522-526): CHC: Out of bounds access happens here.
// Warning 6368: (988-992): CHC: Out of bounds access happens here.
// Warning 6328: (981-998): CHC: Assertion violation happens here.
// Warning 6368: (1009-1013): CHC: Out of bounds access happens here.
