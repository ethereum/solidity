pragma experimental SMTChecker;

contract C
{
	uint[] array;
	uint[][] array2d;
	uint8[] tinyArray;
	function f(
		uint[] storage a,
		uint[] storage b,
		uint[][] storage cc,
		uint8[][] storage dd,
		uint[][][] storage eee
	) internal {
		require(a[0] == 2);
		require(array[0] == 42);
		require(array2d[0][0] == 42);
		require(tinyArray[0] == 42);
		require(cc[0][0] == 42);
		require(dd[0][0] == 42);
		require(eee[0][0][0] == 42);
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
}
// ----
// Warning: (489-506): Assertion violation happens here
// Warning: (553-575): Assertion violation happens here
// Warning: (627-654): Assertion violation happens here
// Warning: (795-817): Assertion violation happens here
// Warning: (957-983): Assertion violation happens here
