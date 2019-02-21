pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint[] storage a, uint[] storage b) internal {
		require(a[0] == 2);
		require(b[0] == 42);
		array[0] = 1;
		// Fails because array == a is possible.
		assert(a[0] == 2);
		// Fails because array == b is possible.
		assert(b[0] == 42);
		assert(array[0] == 1);
	}
}
// ----
// Warning: (226-243): Assertion violation happens here
// Warning: (290-308): Assertion violation happens here
