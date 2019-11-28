pragma experimental SMTChecker;

contract A {
	uint x = 1;
}

contract B is A {
	constructor() public { x = 2; }
}

contract C is A {
	constructor() public { x = 3; }
}

contract D is B, C {
	constructor() public {
		assert(x == 3);
		assert(x == 4);
	}
}
// ----
// Warning: (235-249): Assertion violation happens here
