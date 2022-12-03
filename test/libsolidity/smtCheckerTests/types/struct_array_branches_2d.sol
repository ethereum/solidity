contract C
{
	struct S { uint[][] a; }
	function f(bool b) public pure {
		S memory c;
		c.a = new uint[][](1);
		c.a[0] = new uint[](1);
		c.a[0][0] = 0;
		if (b)
			c.a[0][0] = 1;
		else
			c.a[0][0] = 2;
		assert(c.a[0][0] > 0);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6368: (216-225): CHC: Out of bounds access might happen here.
