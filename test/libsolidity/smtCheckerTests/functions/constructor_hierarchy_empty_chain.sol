pragma experimental SMTChecker;
contract F {
	uint a;
	constructor() {
		a = 2;
	}
}

contract E is F {}
contract D is E {}
contract C is D {}
contract B is C {}

contract A is B {
	constructor(uint x) {
		assert(a == 2);
		assert(a == 3);
	}
}
// ----
// Warning 5667: (194-200): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (224-238): CHC: Assertion violation happens here.\nCounterexample:\na = 2\nx = 0\n\n\nTransaction trace:\nconstructor(0)
