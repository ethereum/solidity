pragma experimental SMTChecker;
contract F {
	uint a;
	constructor(uint x) {
		a = x;
	}
}

abstract contract E is F {}
abstract contract D is E {
	constructor() {
		a = 3;
	}
}
abstract contract C is D {}
contract B is C {
	constructor() F(1) {
		assert(a == 3);
		assert(a == 2);
	}
}

contract A is B {
}
// ----
// Warning 6328: (266-280): Assertion violation happens here
