pragma experimental SMTChecker;

contract C {
	uint x = 2;
}

contract D is C {
	constructor() {
		assert(x == 2);
		assert(x == 3);
	}
}
// ----
// Warning 6328: (117-131): Assertion violation happens here
