pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint x, uint y) public {
		array[x] = 200;
		require(x == y);
		assert(array[y] > 100);
	}
}
