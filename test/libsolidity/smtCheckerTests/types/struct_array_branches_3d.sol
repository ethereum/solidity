contract C
{
	struct S { uint[][][] a; }
	function f(bool b) public pure {
		S memory c;
		c.a = new uint[][][](2);
		assert(c.a.length == 2);
		assert(c.a[0].length == 0);
		c.a[0] = new uint[][](2);
		assert(c.a[0].length == 2);
		assert(c.a[0][0].length == 0);
		c.a[0][0] = new uint[](2);
		// Disabled because of Spacer's seg fault.
		/*
		if (b)
			c.a[0][0][0] = 1;
		else
			c.a[0][0][0] = 2;
		assert(c.a[0][0][0] > 0);
		*/
	}
}
// ====
// SMTEngine: all
// ----
// Warning 5667: (53-59): Unused function parameter. Remove or comment out the variable name to silence this warning.
