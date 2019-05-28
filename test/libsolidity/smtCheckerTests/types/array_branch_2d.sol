pragma experimental SMTChecker;

contract C
{
	uint[][] c;
	function f(bool b) public {
		c[0][0] = 0;
		if (b)
			c[0][0] = 1;
		assert(c[0][0] > 0);
	}
}
// ----
// Warning: (90-97): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (115-122): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (130-149): Assertion violation happens here
