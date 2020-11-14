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
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (370-384): BMC: Assertion violation happens here.
// Warning 4661: (403-418): BMC: Assertion violation happens here.
// Warning 4661: (493-507): BMC: Assertion violation happens here.
// Warning 4661: (526-540): BMC: Assertion violation happens here.
// Warning 4661: (703-717): BMC: Assertion violation happens here.
// Warning 4661: (769-784): BMC: Assertion violation happens here.
// Warning 4661: (860-874): BMC: Assertion violation happens here.
// Warning 4661: (893-907): BMC: Assertion violation happens here.
