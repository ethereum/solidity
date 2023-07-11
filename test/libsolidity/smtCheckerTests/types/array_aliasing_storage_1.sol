contract C
{
	uint[] array;
	uint[][] array2d;
	uint[][][][] array4d;
	uint8[] tinyArray;
	uint8[][][] tinyArray3d;
	function f(
		uint[] storage a,
		uint[] storage b,
		uint[][] storage cc,
		uint8[][] storage dd,
		uint[][][] storage eee
	) internal {
		a[0] = 2;
		array[0] = 42;
		array2d[0][0] = 42;
		tinyArray[0] = 42;
		cc[0][0] = 42;
		dd[0][0] = 42;
		eee[0][0][0] = 42;
		b[0] = 1;
		// Fails because b == a is possible.
		assert(a[0] == 2);
		// Fails because b == array is possible.
		assert(array[0] == 42);
		// Fails because b == array2d[0] is possible.
		assert(array2d[0][0] == 42);
		// Should not fail since knowledge is erased only for uint[].
		assert(tinyArray[0] == 42);
		// Fails because b == cc[0] is possible.
		assert(cc[0][0] == 42);
		// Should not fail since knowledge is erased only for uint[].
		assert(dd[0][0] == 42);
		// Fails because b == ee[0][0] is possible.
		assert(eee[0][0][0] == 42);
		assert(b[0] == 1);
	}

	function g(uint a, uint b, uint c, uint d, uint e) public {
		require(a < array2d.length);
		require(b < array2d.length);
		require(c < array4d.length);
		require(c < array4d[c].length);
		require(d < tinyArray3d.length);
		require(e < array4d.length);
		// Disabled because of Spacer seg fault.
		//f(array2d[a], array2d[b], array4d[c][c], tinyArray3d[d], array4d[e]);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2018: (957-1329): Function state mutability can be restricted to view
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
