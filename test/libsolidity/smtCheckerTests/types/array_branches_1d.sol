pragma experimental SMTChecker;

contract C
{
	function f(bool b, uint[] memory c) public {
		c[0] = 0;
		if (b)
			c[0] = 1;
		else
			c[0] = 2;
		assert(c[0] > 0);
	}
}
// ----
// Warning: (47-168): Function state mutability can be restricted to pure
