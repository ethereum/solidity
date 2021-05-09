contract C {
	uint a;
	modifier n { _; a = 7; }
	constructor(uint x) n {
		a = x;
	}
}

contract A is C {
	modifier m { a = 5; _; }
	constructor() C(2) {
		assert(a == 4);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (156-170): CHC: Assertion violation happens here.\nCounterexample:\na = 7\n\nTransaction trace:\nA.constructor()
