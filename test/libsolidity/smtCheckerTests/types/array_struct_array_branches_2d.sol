pragma experimental SMTChecker;

contract C
{
	struct S { uint[][] a; }
	function f(bool b) public pure {
		S[] memory c;
		c[0].a[0][0] = 0;
		if (b)
			c[0].a[0][0] = 1;
		else
			c[0].a[0][0] = 2;
		assert(c[0].a[0][0] > 0);
	}
}
// ----
