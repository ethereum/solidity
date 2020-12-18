pragma experimental SMTChecker;

contract C
{
	function f(bool b, uint[] memory c) public pure {
		require(c.length <= 2);
		c[0] = 0;
		if (b)
			c[0] = 1;
		assert(c[0] > 0);
	}
}
// ----
// Warning 6328: (159-175): CHC: Assertion violation happens here.\nCounterexample:\n\nb = false\nc = [0, 5]\n\n\nTransaction trace:\nconstructor()\nf(false, [7719, 5])
