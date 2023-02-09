contract C
{
	uint[10][20][30] array;
	function f(uint x, uint y, uint z, uint t, uint w, uint v) public {
		require(x < array.length);
		require(y < array[x].length);
		require(z < array[x][y].length);
		array[x][y][z] = 200;
		require(x == t && y == w && z == v);
		assert(array[t][w][v] > 300);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (268-296): CHC: Assertion violation happens here.
// Info 1391: CHC: 9 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
