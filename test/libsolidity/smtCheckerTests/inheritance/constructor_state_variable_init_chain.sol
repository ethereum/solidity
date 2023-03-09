contract A {
	uint x = 1;
}

contract B is A {
	constructor() { x = 2; }
}

contract C is B {
	constructor() { x = 3; }
}

contract D is C {
	constructor() {
		assert(x == 3);
		assert(x == 2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (178-192): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
