pragma experimental SMTChecker;

contract C {
	uint x = 2;
}

contract D is C {
	constructor() public {
		assert(x == 2);
		assert(x == 3);
	}
}
// ----
// Warning: (124-138): Assertion violation happens here
