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
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (286-300): BMC: Assertion violation happens here.
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
