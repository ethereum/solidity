pragma experimental SMTChecker;

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
		f(array2d[a], array2d[b], array4d[c][c], tinyArray3d[d], array4d[e]);
	}
}
// ----
// Warning: (468-485): Assertion violation happens here
// Warning: (532-554): Assertion violation happens here
// Warning: (606-633): Assertion violation happens here
// Warning: (774-796): Assertion violation happens here
// Warning: (936-962): Assertion violation happens here
// Warning: (468-485): Assertion violation happens here
// Warning: (532-554): Assertion violation happens here
// Warning: (606-633): Assertion violation happens here
// Warning: (774-796): Assertion violation happens here
// Warning: (936-962): Assertion violation happens here
