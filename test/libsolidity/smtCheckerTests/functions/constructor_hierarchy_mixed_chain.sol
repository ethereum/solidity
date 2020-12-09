pragma experimental SMTChecker;
contract F {
	uint a;
	constructor() {
		a = 2;
	}
}

contract E is F {}
contract D is E {
	constructor() {
		a = 3;
	}
}
contract C is D {}
contract B is C {
	constructor() {
		a = 4;
	}
}

contract A is B {
	constructor(uint x) {
		assert(a == 4);
		assert(a == 5);
	}
}
// ----
// Warning 5667: (254-260): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (284-298): CHC: Assertion violation happens here.\nCounterexample:\na = 4\nx = 0\n\n\nTransaction trace:\nconstructor(0)
