pragma experimental SMTChecker;
contract C {
	uint a;
	constructor() {
		a = 2;
	}
}

contract B is C {
}

contract B2 is C {
}

contract A is B, B2 {
	constructor(uint x) {
		assert(a == 2);
		assert(a == 3);
	}
}
// ----
// Warning 5667: (164-170): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (194-208): Assertion violation happens here
