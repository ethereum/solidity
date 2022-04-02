contract C
{
	uint[][][] c;
	constructor() {
		c.push();
		c[0].push();
		c[0][0].push();
	}
	function f(bool b) public {
		c[0][0][0] = 0;
		if (b)
			c[0][0][0] = 1;
		assert(c[0][0][0] < 2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (124-131='c[0][0]'): CHC: Out of bounds access might happen here.
// Warning 6368: (124-134='c[0][0][0]'): CHC: Out of bounds access might happen here.
// Warning 6368: (152-159='c[0][0]'): CHC: Out of bounds access might happen here.
// Warning 6368: (152-162='c[0][0][0]'): CHC: Out of bounds access might happen here.
// Warning 6368: (177-184='c[0][0]'): CHC: Out of bounds access might happen here.
// Warning 6368: (177-187='c[0][0][0]'): CHC: Out of bounds access might happen here.
// Info 1180: Contract invariant(s) for :C:\n!(c.length <= 0)\n
