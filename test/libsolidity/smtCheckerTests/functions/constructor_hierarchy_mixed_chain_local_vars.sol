pragma experimental SMTChecker;
contract F {
	uint a;
	constructor() public {
		uint f = 2;
		a = f;
	}
}

contract E is F {}
contract D is E {
	constructor() public {
		uint d = 3;
		a = d;
	}
}
contract C is D {}
contract B is C {
	constructor() public {
		uint b = 4;
		a = b;
	}
}

contract A is B {
	constructor(uint x) public {
		uint a1 = 4;
		uint a2 = 5;
		assert(a == a1);
		assert(a == a2);
	}
}
// ----
// Warning: (317-323): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (385-400): Assertion violation happens here
