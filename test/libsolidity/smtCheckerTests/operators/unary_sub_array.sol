pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint x) public {
		array[x] = 5;
		uint a = --array[x];
		assert(array[x] == 4);
		assert(a == 4);
		uint b = array[x]--;
		assert(array[x] == 3);
		// Should fail.
		assert(b > 4);
	}
}
// ----
// Warning: (240-253): Assertion violation happens here
