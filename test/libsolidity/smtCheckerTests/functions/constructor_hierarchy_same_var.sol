pragma experimental SMTChecker;
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
// ----
// Warning 6328: (134-148): CHC: Assertion violation happens here.\nCounterexample:\na = 2\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (152-168): CHC: Assertion violation happens here.\nCounterexample:\na = 2\n\n\n\nTransaction trace:\nconstructor()
