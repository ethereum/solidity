contract C
{
	uint[][] c;
	constructor() {
		c.push().push();
	}
	function f(bool b) public {
		// Disabled because of Spacer nondeterminism.
		/*
		c[0][0] = 0;
		if (b)
			c[0][0] = 1;
		assert(c[0][0] > 0);
		*/
	}
}
// ====
// SMTEngine: all
// ----
