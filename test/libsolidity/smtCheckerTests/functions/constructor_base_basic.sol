pragma experimental SMTChecker;

contract A {
	uint x;
	constructor() {
		x = 2;
	}
}

contract B is A {
	constructor() A() {
		x = 3;
	}
}
// ----
