pragma experimental SMTChecker;

contract A {
	uint x = 2;
}

contract B is A {
}

contract C is A {
}

contract D is B, C {
	constructor() public {
		assert(x == 2);
		assert(x == 3);
	}
}
// ----
// Warning: (169-183): Assertion violation happens here
