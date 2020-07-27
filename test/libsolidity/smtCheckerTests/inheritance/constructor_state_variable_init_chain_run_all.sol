pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) {
		a = x;
	}
}

abstract contract B is C {
	uint b;
	constructor(uint x) {
		b = a + x;
	}
}

contract A is B {
	constructor(uint x) B(x) C(x + 2) {
		assert(a == x + 2);
		assert(b == x + x + 2);
		assert(a == x + 5);
	}
}

// ----
// Warning 6328: (275-293): Assertion violation happens here
// Warning 4144: (157-162): Underflow (resulting value less than 0) happens here
// Warning 2661: (157-162): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (216-221): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (157-162): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (239-244): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (261-266): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (261-270): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (287-292): Overflow (resulting value larger than 2**256 - 1) happens here
