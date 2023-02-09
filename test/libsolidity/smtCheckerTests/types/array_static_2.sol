contract C
{
	uint[10][20] array;
	function f(uint x, uint y, uint z, uint t) public view {
		require(x < array.length);
		require(y < array[x].length);
		require(array[x][y] == 200);
		require(x == z && y == t);
		assert(array[z][t] > 100);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
