pragma experimental SMTChecker;
contract F {
	uint a;
	constructor() public {
		a = 2;
	}
}

contract E is F {}
contract D is E {
	constructor() public {
		a = 3;
	}
}
contract C is D {}
contract B is C {
	constructor() public {
		a = 4;
	}
}

contract A is B {
	constructor(uint x) public {
		assert(a == 4);
		assert(a == 5);
	}
}
// ----
// Warning: (275-281): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (312-326): Assertion violation happens here
