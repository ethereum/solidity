pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint x, uint y) public {
		array[x] = 200;
		require(x == y);
		assert(array[y] > 300);
	}
}
// ----
// Warning: (137-159): Assertion violation happens here
