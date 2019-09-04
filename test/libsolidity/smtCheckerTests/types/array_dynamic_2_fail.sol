pragma experimental SMTChecker;

contract C
{
	uint[][] array;
	function f(uint x, uint y, uint z, uint t) public view {
		// TODO change to = 200 when 2d assignments are supported.
		require(array[x][y] < 200);
		require(x == z && y == t);
		assert(array[z][t] > 300);
	}
}
// ----
// Warning: (243-268): Assertion violation happens here
