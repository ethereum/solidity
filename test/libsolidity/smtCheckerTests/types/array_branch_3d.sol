pragma experimental SMTChecker;

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
// ----
// Warning 6368: (157-164): CHC: Out of bounds access might happen here.
// Warning 6368: (157-167): CHC: Out of bounds access might happen here.
// Warning 6368: (185-192): CHC: Out of bounds access might happen here.
// Warning 6368: (185-195): CHC: Out of bounds access might happen here.
// Warning 6368: (210-217): CHC: Out of bounds access might happen here.
// Warning 6368: (210-220): CHC: Out of bounds access might happen here.
// Warning 6328: (203-225): CHC: Assertion violation happens here.\nCounterexample:\nc = [[[0]]]\nb = false\n\nTransaction trace:\nC.constructor()\nState: c = [[[0]]]\nC.f(false)
