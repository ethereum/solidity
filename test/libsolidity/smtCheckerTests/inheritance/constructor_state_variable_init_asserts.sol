contract A {
	int x;
	constructor (int a) { x = a;}
}

contract B is A {
	int y;
	constructor(int a) A(-a) {
		if (a > 0) {
			y = 2;
		}
		else {
			y = 4;
		}
	}
}

contract C is B {
	constructor(int a) B(a) {
		assert(y != 3); // should hold
		assert(y == 4); // should fail
		if (a > 0) {
			assert(x < 0 && y == 2); // should hold
			assert(x < 0 && y == 4); // should fail
		}
		else {
			assert(x >= 0 && y == 4); // should hold
			assert(x >= 0 && y == 2); // should fail
			assert(x > 0); // should fail
		}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (247-261): CHC: Assertion violation happens here.\nCounterexample:\ny = 2, x = (- 1)\na = 1\n\nTransaction trace:\nC.constructor(1)
// Warning 6328: (339-362): CHC: Assertion violation happens here.\nCounterexample:\ny = 2, x = (- 1)\na = 1\n\nTransaction trace:\nC.constructor(1)
// Warning 6328: (439-463): CHC: Assertion violation happens here.\nCounterexample:\ny = 4, x = 0\na = 0\n\nTransaction trace:\nC.constructor(0)
// Warning 6328: (483-496): CHC: Assertion violation happens here.\nCounterexample:\ny = 4, x = 0\na = 0\n\nTransaction trace:\nC.constructor(0)
