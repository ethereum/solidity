contract C
{
	function f(bool b, uint[] memory c) public pure {
		require(c.length > 0);
		c[0] = 0;
		if (b)
			c[0] = 1;
		else
			c[0] = 2;
		assert(c[0] > 0);
	}
}
// ====
// SMTEngine: all
// ----
