pragma experimental SMTChecker;

contract C
{
	function f(uint[] memory array, uint x, uint y) public pure {
		array[x] = 200;
		require(x == y);
		assert(array[y] > 300);
	}
}
// ----
// Warning: (148-170): Assertion violation happens here
