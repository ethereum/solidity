pragma experimental SMTChecker;

contract A {
	int x;
	constructor (int a) { x = a; }
}

contract Z {
	int z;
	constructor(int _z) {
		z = _z;
	}
}

contract B is A, Z {
	constructor(int b) A(b) Z(x) {
		assert(x == b);
		assert(z == 0);
	}
}

contract F is Z, A {
	constructor(int b) Z(x) A(b) {
		assert(x == b);
		assert(z == 0);
	}
}

contract C is B {
	constructor(int c) B(-c) {
		if (x > 0) {
			assert(c < 0); // should hold
			assert(c >= 0); // should fail
		}
		else {
			assert(c < 0); // should fail
			assert(c >= 0); // should hold
		}
	}
}
// ----
// Warning 6328: (436-450): CHC: Assertion violation happens here.\nCounterexample:\nz = 0, x = 1\nc = (- 1)\n\n\nTransaction trace:\nconstructor((- 1))
// Warning 6328: (483-496): CHC: Assertion violation happens here.\nCounterexample:\nz = 0, x = 0\nc = 0\n\n\nTransaction trace:\nconstructor(0)
