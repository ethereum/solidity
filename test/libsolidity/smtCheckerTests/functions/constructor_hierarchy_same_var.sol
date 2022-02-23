contract C {
	uint a;
	constructor(uint x) {
		a = x;
	}
}

contract A is C {
	constructor() C(2) {
		assert(a == 0);
		assert(C.a == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (102-116): CHC: Assertion violation happens here.\nCounterexample:\na = 2\n\nTransaction trace:\nA.constructor()
// Warning 6328: (120-136): CHC: Assertion violation happens here.\nCounterexample:\na = 2\n\nTransaction trace:\nA.constructor()
