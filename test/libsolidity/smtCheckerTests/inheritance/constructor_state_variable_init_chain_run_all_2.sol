pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) public {
		a = x;
	}
}

contract B is C {
	uint b;
	constructor(uint x) public {
		b = x + 10;
	}
}

contract A is B {
	constructor(uint x) B(x) C(x + 2) public {
		assert(a == x + 2);
		assert(b == x + 10);
		assert(b == x + 5);
	}
}

// ----
// Warning: (162-168): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (285-303): Assertion violation happens here
