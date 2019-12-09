pragma experimental SMTChecker;

contract A {
	uint x = 1;
}

contract B is A {
	constructor() public { x = 2; }
}

contract C is B {
	constructor() public { x = 3; }
}

contract D is C {
	constructor() public {
		assert(x == 3);
		assert(x == 2);
	}
}
// ----
// Warning: (232-246): Assertion violation happens here
