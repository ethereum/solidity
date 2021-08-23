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
		// Disabled because of Spacer nondeterminism.
		//f(array2d[0], c);
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
// Warning 5667: (125-140): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 2018: (106-254): Function state mutability can be restricted to view
