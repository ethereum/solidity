pragma experimental SMTChecker;
contract F {
	uint a;
	constructor(uint x) public {
		a = x;
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
	constructor() F(1) public {
		assert(a == 3);
		assert(a == 2);
	}
}

contract A is B {
}
// ----
// Warning: (260-274): Assertion violation happens here
// Warning: (260-274): Assertion violation happens here
