contract C
{
	uint[10][20] array;
	function f(uint x, uint y, uint z, uint t) public view {
		require(x < array.length);
		require(y < array[x].length);
		require(array[x][y] < 200);
		require(x == z && y == t);
		assert(array[z][t] > 300);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (214-239): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
