contract C
{
	uint[][][] c;
	constructor() {
		c.push();
		c[0].push();
		c[0][0].push();
	}
	function f(bool b) public {
		if (b)
			c[0][0][0] = 1;
		assert(c[0][0][0] > 0);
	}
}
// ====
// SMTEngine: chc
// SMTSolvers: eld
// SMTTargets: assert
// ----
// Warning 6328: (152-174): CHC: Assertion violation happens here.\nCounterexample:\nc = [[[0]]]\nb = false\n\nTransaction trace:\nC.constructor()\nState: c = [[[0]]]\nC.f(false)
