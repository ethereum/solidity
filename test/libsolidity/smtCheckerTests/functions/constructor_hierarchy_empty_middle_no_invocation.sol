pragma experimental SMTChecker;
contract C {
	uint a;
	constructor() public {
		a = 2;
	}
}

contract B is C {
}

contract A is B {
	constructor(uint x) public {
		assert(a == 3);
	}
}
// ----
// Warning: (145-151): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (164-178): Assertion violation happens here
