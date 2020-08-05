pragma experimental SMTChecker;
contract C {
	uint a;
	constructor() {
		a = 2;
	}
}

contract B is C {
}

contract A is B {
	constructor(uint x) B() {
		assert(a == 2);
		assert(a == 3);
	}
}
// ----
// Warning 5667: (138-144): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (172-186): Assertion violation happens here
