pragma experimental SMTChecker;
contract C {
	uint a;
	constructor() public {
		a = 2;
	}
}

contract B is C {
}

contract B2 is C {
}

contract A is B, B2 {
	constructor(uint x) public {
		assert(a == 2);
		assert(a == 3);
	}
}
// ----
// Warning: (171-177): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (208-222): Assertion violation happens here
