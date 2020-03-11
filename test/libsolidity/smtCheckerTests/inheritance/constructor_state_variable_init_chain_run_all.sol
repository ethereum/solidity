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
// Warning: (171-176): Underflow (resulting value less than 0) happens here
// Warning: (171-176): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (230-235): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (171-176): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (260-265): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (282-287): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (282-291): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (308-313): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (296-314): Assertion violation happens here
