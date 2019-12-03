pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) public {
		a = x;
	}
}

contract B1 is C {
	constructor(uint x) public {
		a = x;
	}
}

contract B2 is C {
	constructor(uint x) C(x + 2) public {
		a = x;
	}
}

contract A is B2, B1 {
	constructor(uint x) B2(x) B1(x) public {
		assert(a == x);
		assert(a == x + 1);
	}
}
// ----
// Warning: (205-210): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (321-339): Assertion violation happens here
