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
// Warning 6328: (178-192): CHC: Assertion violation happens here.\nCounterexample:\nx = 3\n\nTransaction trace:\nD.constructor()
