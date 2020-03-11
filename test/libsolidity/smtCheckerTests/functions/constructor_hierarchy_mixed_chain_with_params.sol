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
// Warning: (261-266): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (261-266): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (356-370): Assertion violation happens here
