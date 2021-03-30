pragma experimental SMTChecker;

contract C
{
	uint[] b;
	uint[] d;
	uint[][] array2d;
	function p() public {
		array2d.push().push();
	}
	function g(uint x, uint[] memory c) public {
		require(x < array2d.length);
		f(array2d[x], c);
	}
	function f(uint[] storage a, uint[] memory c) internal {
		// Accesses are safe but oob is reported because of aliasing.
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
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6368: (362-366): CHC: Out of bounds access happens here.
// Warning 6368: (375-379): CHC: Out of bounds access happens here.
// Warning 6368: (388-392): CHC: Out of bounds access happens here.
// Warning 6368: (400-404): CHC: Out of bounds access happens here.
// Warning 6368: (523-527): CHC: Out of bounds access happens here.
// Warning 6368: (725-729): CHC: Out of bounds access happens here.
// Warning 6328: (718-735): CHC: Assertion violation happens here.
// Warning 6368: (829-833): CHC: Out of bounds access happens here.
