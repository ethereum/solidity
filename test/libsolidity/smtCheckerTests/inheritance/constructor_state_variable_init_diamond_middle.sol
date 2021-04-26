contract A {
	uint x = 1;
}

contract B is A {
	constructor() {
		assert(x == 1);
		x = 2;
	}
}

contract C is A {
	constructor() {
		assert(x == 1);
		x = 3;
	}
}

contract D is B, C {
	constructor() {
		assert(x == 3);
		assert(x == 4);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (223-237): CHC: Assertion violation happens here.\nCounterexample:\nx = 3\n\nTransaction trace:\nD.constructor()
// Warning 6328: (134-148): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nD.constructor()
