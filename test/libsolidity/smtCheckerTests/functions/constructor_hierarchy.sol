pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) public {
		a = x;
	}
}

contract A is C {
	constructor() C(2) public {
		assert(a == 2);
		assert(a == 3);
	}
}
// ----
// Warning: (166-180): Assertion violation happens here
