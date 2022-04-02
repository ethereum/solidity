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
// ====
// SMTEngine: bmc
// ----
// Warning 5740: (119-124='x = 3'): Unreachable code.
// Warning 4661: (277-291='assert(x == 3)'): BMC: Assertion violation happens here.
// Warning 4661: (310-324='assert(x == 2)'): BMC: Assertion violation happens here.
// Warning 4661: (343-357='assert(x == 1)'): BMC: Assertion violation happens here.
