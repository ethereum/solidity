pragma experimental SMTChecker;

contract A {
	uint x = 1;
}

contract B is A {
	constructor() { x = 2; }
}

contract C is B {
}

contract D is C {
	constructor() {
		assert(x == 2);
		assert(x == 3);
	}
}
// ----
// Warning 6328: (185-199): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\n\n\nTransaction trace:\nconstructor()
