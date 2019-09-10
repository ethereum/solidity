pragma experimental SMTChecker;

contract C
{
	uint[] a;
	function g(uint x, uint y) public {
		require(x != y);
		(a[x], a[y]) = (2, 4);
		assert(a[x] == 2);
		assert(a[y] == 4);
	}
}
