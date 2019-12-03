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
