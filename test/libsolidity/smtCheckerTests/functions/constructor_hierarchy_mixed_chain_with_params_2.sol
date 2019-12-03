pragma experimental SMTChecker;
contract F {
	uint a;
	constructor(uint x) public {
		a = x;
	}
}

abstract contract E is F {}
abstract contract D is E {
	constructor() public {
		a = 3;
	}
}
abstract contract C is D {}
contract B is C {
	constructor() F(1) public {
		assert(a == 3);
		assert(a == 2);
	}
}

contract A is B {
}
// ----
// Warning: (287-301): Assertion violation happens here
// Warning: (287-301): Assertion violation happens here
