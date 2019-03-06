pragma experimental SMTChecker;

contract C
{
	uint[10][20] array;
	function f(uint x, uint y, uint z, uint t) public view {
		require(array[x][y] == 200);
		require(x == z && y == t);
		assert(array[z][t] > 300);
	}
}
// ----
// Warning: (187-212): Assertion violation happens here
