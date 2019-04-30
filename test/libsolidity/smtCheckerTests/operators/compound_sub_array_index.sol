pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint x, uint p) public {
		require(x < 100);
		require(array[p] == 200);
		array[p] -= array[p] - x;
		assert(array[p] >= 0);
		assert(array[p] < 90);
	}
}
// ----
// Warning: (201-222): Assertion violation happens here
