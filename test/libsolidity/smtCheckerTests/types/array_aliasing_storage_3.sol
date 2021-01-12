pragma experimental SMTChecker;

contract C
{
	uint[][] array2d;
	function g(uint x, uint y, uint[] memory c) public {
		f(array2d[x], array2d[y], c);
	}
	function f(uint[] storage a, uint[] storage b, uint[] memory c) internal {
		uint[] memory d = c;
		c[0] = 42;
		a[0] = 2;
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
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 2072: (232-247): Unused local variable.
// Warning 6328: (679-696): CHC: Assertion violation happens here.
