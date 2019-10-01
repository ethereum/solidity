pragma experimental SMTChecker;

contract C
{
	uint[][][] array;
	function f(uint x, uint y, uint z, uint t, uint w, uint v) public view {
		// TODO change to = 200 when 3d assignments are supported.
		require(array[x][y][z] < 200);
		require(x == t && y == w && z == v);
		assert(array[t][w][v] > 300);
	}
}
// ----
// Warning: (274-302): Assertion violation happens here
