pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) public {
		a = x;
	}
}

abstract contract B is C {
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
// Warning 4144: (171-176): Underflow (resulting value less than 0) happens here
// Warning 2661: (171-176): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (230-235): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (171-176): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (260-265): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (282-287): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (282-291): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (308-313): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 4661: (296-314): Assertion violation happens here
