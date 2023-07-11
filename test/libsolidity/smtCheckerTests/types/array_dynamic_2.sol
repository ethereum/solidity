contract C
{
	uint[][] array;
	function f(uint x, uint y, uint z, uint t) public view {
		require(array[x][y] == 200);
		require(x == z && y == t);
		assert(array[z][t] > 100);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (98-106): CHC: Out of bounds access happens here.
// Warning 6368: (98-109): CHC: Out of bounds access happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
