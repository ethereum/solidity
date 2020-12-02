pragma experimental SMTChecker;

contract A {
	uint x = 1;
}

contract B is A {
	constructor() {
		assert(x == 1);
		x = 2;
	}
}

contract C is A {
	constructor() {
		assert(x == 1);
		x = 3;
	}
}

contract D is B, C {
	constructor() {
		assert(x == 3);
		assert(x == 4);
	}
}
// ----
// Warning 6328: (167-181): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (256-270): CHC: Assertion violation happens here.\nCounterexample:\nx = 3\n\n\n\nTransaction trace:\nconstructor()
