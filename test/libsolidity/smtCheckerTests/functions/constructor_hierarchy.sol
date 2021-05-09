contract C {
	uint a;
	constructor(uint x) {
		a = x;
	}
}

contract A is C {
	constructor() C(2) {
		assert(a == 2);
		assert(a == 3);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (120-134): CHC: Assertion violation happens here.\nCounterexample:\na = 2\n\nTransaction trace:\nA.constructor()
