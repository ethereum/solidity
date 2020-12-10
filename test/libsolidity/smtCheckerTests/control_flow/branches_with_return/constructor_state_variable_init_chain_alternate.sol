pragma experimental SMTChecker;

contract A {
	uint x = 1;
}

contract B is A {
	constructor(int a) {
		if (a > 0) {
			x = 2;
			return;
		}
		x = 3;
	}
}

abstract contract C is B {
}

contract D is C {
	constructor(int a) B(a) {
		assert(a > 0 || x == 3); // should hold
		assert(a <= 0 || x == 2); // should hold
		assert(x == 1); // should fail
	}
}
// ----
// Warning 6328: (319-333): CHC: Assertion violation happens here.\nCounterexample:\nx = 3\na = 0\n\n\nTransaction trace:\nconstructor(0)
