contract C {
	uint x = address(this).balance;
	constructor() {
		assert(x == 0); // should fail because there might be funds from before deployment
		assert(x > 0); // should fail
	}
	function f() public view {
		assert(x == 0); // should fail because there might be funds from before deployment
		assert(x > 0); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (65-79='assert(x == 0)'): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\nTransaction trace:\nC.constructor()
// Warning 6328: (150-163='assert(x > 0)'): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()
// Warning 6328: (213-227='assert(x == 0)'): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\nTransaction trace:\nC.constructor()\nState: x = 1\nC.f()
// Warning 6328: (298-311='assert(x > 0)'): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f()
