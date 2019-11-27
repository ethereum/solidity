pragma experimental SMTChecker;

contract A {
	uint x;
	constructor() public {
		x = 2;
	}
}

contract B is A {
	constructor() A() public {
		x = 3;
	}
}
// ----
// Warning: (56-90): Assertion checker does not yet support constructors.
// Warning: (113-151): Assertion checker does not yet support constructors.
