pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint[] memory a, uint[] memory b) internal view {
		require(array[0] == 42);
		uint[] storage c = array;
		require(a[0] == 2);
		b[0] = 1;
		// Erasing knowledge about memory references should not
		// erase knowledge about state variables.
		assert(array[0] == 42);
		// Erasing knowledge about memory references should not
		// erase knowledge about storage references.
		assert(c[0] == 42);
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (469-486): Assertion violation happens here
