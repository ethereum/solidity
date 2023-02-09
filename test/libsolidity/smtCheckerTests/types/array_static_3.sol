contract C
{
	uint[10][20][30] array;
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
// Info 1391: CHC: 10 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
