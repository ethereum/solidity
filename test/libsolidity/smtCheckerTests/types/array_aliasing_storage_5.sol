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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6368: (186-196): CHC: Out of bounds access might happen here.
// Warning 6368: (329-333): CHC: Out of bounds access happens here.
// Warning 6368: (342-346): CHC: Out of bounds access happens here.
// Warning 6368: (355-359): CHC: Out of bounds access happens here.
// Warning 6368: (367-371): CHC: Out of bounds access happens here.
// Warning 6368: (490-494): CHC: Out of bounds access happens here.
// Warning 6368: (692-696): CHC: Out of bounds access happens here.
// Warning 6328: (685-702): CHC: Assertion violation happens here.
// Warning 6368: (796-800): CHC: Out of bounds access happens here.
