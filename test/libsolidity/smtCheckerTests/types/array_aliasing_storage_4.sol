contract C
{
	uint[] array;
	uint[][] array2d;
	function p() public {
		array.push();
		array2d.push().push();
	}
	function f(uint[] storage a, uint[] storage b) internal {
		require(a.length > 0);
		require(b.length > 0);
		a[0] = 2;
		// Accesses are safe but oob is reported because of aliasing.
		b[0] = 42;
		array[0] = 1;
		// Fails because array == a is possible.
		assert(a[0] == 2);
		// Fails because array == b is possible.
		assert(b[0] == 42);
		assert(array[0] == 1);
	}
	function g(uint x, uint y) public {
		require(x < array2d.length);
		require(y < array2d.length);
		// Disabled because of Spacer nondeterminism.
		//f(array2d[x], array2d[y]);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2018: (486-665): Function state mutability can be restricted to view
