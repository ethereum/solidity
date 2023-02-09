contract C {
	uint[][] c;

	function f() public {
		require(c.length == 0);
		c.push().push() = 2;
		assert(c.length == 1);
		assert(c[0].length == 1);
		assert(c[0][0] == 2);
	}

	function g() public {
		c.push().push() = 2;
		assert(c.length > 0);
		assert(c[c.length - 1].length == 1);
		assert(c[c.length - 1][c[c.length - 1].length - 1] == 2);
		// Fails
		assert(c[c.length - 1][c[c.length - 1].length - 1] == 200);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (362-420): CHC: Assertion violation happens here.
// Info 1391: CHC: 23 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
