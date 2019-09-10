pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint x, uint p) public {
		require(x < 10);
		require(array[p] == 10);
		array[p] *= array[p] + x;
		assert(array[p] <= 190);
		assert(array[p] < 50);
	}
}
// ----
// Warning: (201-222): Assertion violation happens here
