pragma experimental SMTChecker;

contract C
{
	uint[] a;
	function g(uint x, uint y) public {
		require(x != y);
		(, a[y]) = (2, 4);
		assert(a[x] == 2);
		assert(a[y] == 4);
	}
}
// ----
// Warning 6328: (136-153): CHC: Assertion violation happens here.\nCounterexample:\na = []\nx = 39\ny = 0\n\n\nTransaction trace:\nconstructor()\nState: a = []\ng(39, 0)
