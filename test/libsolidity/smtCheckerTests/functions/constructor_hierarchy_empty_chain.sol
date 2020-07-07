pragma experimental SMTChecker;
contract F {
	uint a;
	constructor() public {
		a = 2;
	}
}

contract E is F {}
contract D is E {}
contract C is D {}
contract B is C {}

contract A is B {
	constructor(uint x) public {
		assert(a == 2);
		assert(a == 3);
	}
}
// ----
// Warning 5667: (201-207): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 4661: (238-252): Assertion violation happens here
