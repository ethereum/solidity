contract C
{
	function f(bool b, uint[] memory c) public pure {
		require(c.length >= 1 && c.length <= 2);
		c[0] = 0;
		if (b)
			c[0] = 1;
		assert(c[0] > 0);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (143-159): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
