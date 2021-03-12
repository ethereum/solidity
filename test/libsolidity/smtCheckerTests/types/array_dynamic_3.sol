contract C
{
	uint[][][] array;
	function f(uint x, uint y, uint z, uint t, uint w, uint v) public view {
		require(x < array.length);
		require(y < array[x].length);
		require(z < array[x][y].length);
		require(array[x][y][z] == 200);
		require(x == t && y == w && z == v);
		assert(array[t][w][v] > 100);
	}
}
// ====
// SMTEngine: all
// ----
