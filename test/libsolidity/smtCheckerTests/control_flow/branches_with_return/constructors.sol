pragma experimental SMTChecker;

contract B {
	int x;
	constructor(int b) {
		if (b > 0) {
			x = 1;
			return;
		}
		else {
			x = 2;
			return;
		}
		x = 3; // dead code
	}
}

contract C is B {
	constructor(int a) B(a) {
		assert(a > 0 || x == 2); // should hold
		assert(a <= 0 || x == 1); // should hold
		assert(x == 3); // should fail
		assert(x == 2); // should fail
		assert(x == 1); // should fail
	}
}
// ----
// Warning 5740: (152-157): Unreachable code.
// Warning 6328: (310-324): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\na = 1\n\n\nTransaction trace:\nconstructor(1)
// Warning 6328: (343-357): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\na = 1\n\n\nTransaction trace:\nconstructor(1)
// Warning 6328: (376-390): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\na = 0\n\n\nTransaction trace:\nconstructor(0)
