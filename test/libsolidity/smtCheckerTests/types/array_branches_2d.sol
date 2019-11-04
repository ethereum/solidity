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
