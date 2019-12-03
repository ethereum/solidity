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
		b = a + x;
	}
}

contract A is B {
	constructor(uint x) B(x) C(x + 2) public {
		assert(a == x + 2);
		assert(b == x + x + 2);
		assert(a == x + 5);
	}
}

// ----
// Warning: (162-167): Underflow (resulting value less than 0) happens here
// Warning: (162-167): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (287-305): Assertion violation happens here
