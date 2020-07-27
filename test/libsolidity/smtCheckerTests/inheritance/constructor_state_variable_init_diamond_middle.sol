pragma experimental SMTChecker;

contract A {
	uint x = 1;
}

contract B is A {
	constructor() { x = 2; }
}

contract C is A {
	constructor() { x = 3; }
}

contract D is B, C {
	constructor() {
		assert(x == 3);
		assert(x == 4);
	}
}
// ----
// Warning 6328: (214-228): Assertion violation happens here
