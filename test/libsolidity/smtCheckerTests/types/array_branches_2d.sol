pragma experimental SMTChecker;

contract C
{
	uint[][] c;
	function f(bool b) public {
		c[0][0] = 0;
		if (b)
			c[0][0] = 1;
		else
			c[0][0] = 2;
		assert(c[0][0] > 0);
	}
}
// ----
// Warning: (90-97): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (115-122): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (138-145): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (153-172): Assertion violation happens here
