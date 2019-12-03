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
	constructor(uint x) F(x + 1) public {
	}
}

contract A is B {
	constructor(uint x) B(x) public {
		assert(a == 3);
		assert(a == 4);
	}
}
// ----
// Warning: (234-239): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (329-343): Assertion violation happens here
