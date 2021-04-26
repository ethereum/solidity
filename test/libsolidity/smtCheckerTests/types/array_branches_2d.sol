contract C
{
	uint[][] c;
	function p() public {
		c.push().push();
	}
	function f(bool b) public {
		require(c.length > 0);
		require(c[0].length > 0);
		c[0][0] = 0;
		if (b)
			c[0][0] = 1;
		else
			c[0][0] = 2;
		assert(c[0][0] > 0);
	}
}
// ====
// SMTEngine: all
// ----
