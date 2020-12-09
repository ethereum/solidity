pragma experimental SMTChecker;

contract A {
	uint x = 2;
}

contract B is A {
}

contract C is A {
}

contract D is B, C {
	constructor() {
		assert(x == 2);
		assert(x == 3);
	}
}
// ----
// Warning 6328: (162-176): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\n\n\nTransaction trace:\nconstructor()
