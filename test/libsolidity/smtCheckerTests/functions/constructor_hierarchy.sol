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
// Warning 4661: (152-166): Assertion violation happens here
