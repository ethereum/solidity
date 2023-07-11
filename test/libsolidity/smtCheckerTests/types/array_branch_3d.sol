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
		assert(c[0][0][0] > 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (124-131): CHC: Out of bounds access might happen here.
// Warning 6368: (124-134): CHC: Out of bounds access might happen here.
// Warning 6368: (152-159): CHC: Out of bounds access might happen here.
// Warning 6368: (152-162): CHC: Out of bounds access might happen here.
// Warning 6368: (177-184): CHC: Out of bounds access might happen here.
// Warning 6368: (177-187): CHC: Out of bounds access might happen here.
// Warning 6328: (170-192): CHC: Assertion violation happens here.
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
