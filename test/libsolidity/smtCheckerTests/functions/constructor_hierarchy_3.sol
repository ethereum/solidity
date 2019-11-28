pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) public {
		a = x;
	}
}

contract B is C {
	constructor(uint x) public {
		a = x;
	}
}

contract A is B {
	constructor(uint x) B(x) C(x + 2) public {
		assert(a == x);
		assert(a == x + 1);
	}
}
// ----
// Warning: (244-262): Assertion violation happens here
