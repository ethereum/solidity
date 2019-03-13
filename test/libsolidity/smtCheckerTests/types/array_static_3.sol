pragma experimental SMTChecker;

contract C
{
	uint[10][20][30] array;
	function f(uint x, uint y, uint z, uint t, uint w, uint v) public view {
		require(array[x][y][z] == 200);
		require(x == t && y == w && z == v);
		assert(array[t][w][v] > 100);
	}
}
