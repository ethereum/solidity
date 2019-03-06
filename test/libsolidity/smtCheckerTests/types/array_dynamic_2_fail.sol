pragma experimental SMTChecker;

contract C
{
	uint[][] array;
	function f(uint x, uint y, uint z, uint t) public view {
		require(array[x][y] == 200);
		require(x == z && y == t);
		assert(array[z][t] > 300);
	}
}
// ----
// Warning: (183-208): Assertion violation happens here
