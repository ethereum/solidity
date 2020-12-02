pragma experimental SMTChecker;
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
// ----
// Warning 6328: (152-166): CHC: Assertion violation happens here.\nCounterexample:\na = 2\n\n\n\nTransaction trace:\nconstructor()
