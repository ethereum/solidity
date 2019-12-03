pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) public {
		a = x;
	}
}

contract B1 is C {
	uint b1;
	constructor(uint x) public {
		b1 = x + a;
	}
}

contract B2 is C {
	uint b2;
	constructor(uint x) C(x + 2) public {
		b2 = x + a;
	}
}

contract A is B2, B1 {
	constructor(uint x) B2(x) B1(x) public {
		assert(b1 == b2);
		assert(b1 != b2);
	}
}
// ----
// Warning: (165-170): Underflow (resulting value less than 0) happens here
// Warning: (165-170): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (230-235): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (253-258): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (353-369): Assertion violation happens here
