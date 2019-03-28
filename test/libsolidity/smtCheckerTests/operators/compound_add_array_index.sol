pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint x, uint p) public {
		require(x < 100);
		require(array[p] == 100);
		array[p] += array[p] + x;
		assert(array[p] < 300);
		assert(array[p] < 110);
	}
}
// ----
// Warning: (202-224): Assertion violation happens here
