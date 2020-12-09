pragma experimental SMTChecker;

contract A {
	int x;
}

contract B is A {
	int y;
	constructor (int a) {
		if (a >= 0) {
			y = 1;
			return;
		}
		x = 1;
		y = 2;
	}
}

contract C is A {
	int z;
	constructor (int a) {
		if (a >= 0) {
			z = 1;
			return;
		}
		x = -1;
		z = 2;
	}
}

contract D1 is B, C {
	constructor() B(1) C(1) {
		assert(x == 0); // should hold
		assert(x == 1); // should fail
		assert(x == -1); // should fail
	}
}

contract D2 is B, C {
	constructor() B(1) C(-1) {
		assert(x == 0); // should fail
		assert(x == 1); // should fail
		assert(x == -1); // should hold (constructor of C is executed AFTER constructor of B)
	}
}

contract D3 is B, C {
	constructor() B(-1) C(1) {
		assert(x == 0); // should fail
		assert(x == 1); // should hold
		assert(x == -1); // should fail
	}
}

contract D4 is B, C {
	constructor() B(-1) C(-1) {
		assert(x == 0); // should fail
		assert(x == 1); // should fail
		assert(x == -1); // should hold (constructor of C is executed AFTER constructor of B)
	}
}
// ----
// Warning 6328: (370-384): CHC: Assertion violation happens here.\nCounterexample:\nz = 1, y = 1, x = 0\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (403-418): CHC: Assertion violation happens here.\nCounterexample:\nz = 1, y = 1, x = 0\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (493-507): CHC: Assertion violation happens here.\nCounterexample:\nz = 2, y = 1, x = (- 1)\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (526-540): CHC: Assertion violation happens here.\nCounterexample:\nz = 2, y = 1, x = (- 1)\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (703-717): CHC: Assertion violation happens here.\nCounterexample:\nz = 1, y = 2, x = 1\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (769-784): CHC: Assertion violation happens here.\nCounterexample:\nz = 1, y = 2, x = 1\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (860-874): CHC: Assertion violation happens here.\nCounterexample:\nz = 2, y = 2, x = (- 1)\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (893-907): CHC: Assertion violation happens here.\nCounterexample:\nz = 2, y = 2, x = (- 1)\n\n\n\nTransaction trace:\nconstructor()
