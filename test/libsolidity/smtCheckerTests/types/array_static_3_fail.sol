pragma experimental SMTChecker;

contract C
{
	uint[10][20][30] array;
	function f(uint x, uint y, uint z, uint t, uint w, uint v) public view {
		// TODO change to = 200 when 3d assignments are supported.
		require(array[x][y][z] < 200);
		require(x == t && y == w && z == v);
		assert(array[t][w][v] > 300);
	}
}
// ----
// Warning: (280-308): Assertion violation happens here
