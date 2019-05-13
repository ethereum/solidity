pragma experimental SMTChecker;

contract C
{
	uint[][][] c;
	function f(bool b) public {
		c[0][0][0] = 0;
		if (b)
			c[0][0][0] = 1;
		else
			c[0][0][0] = 2;
		assert(c[0][0][0] > 0);
	}
}
// ----
// Warning: (92-102): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (120-130): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (146-156): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (164-186): Assertion violation happens here
