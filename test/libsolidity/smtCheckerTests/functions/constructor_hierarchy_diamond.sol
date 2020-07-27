pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) {
		a = x;
	}
}

abstract contract B1 is C {
	constructor(uint x) {
		a = x;
	}
}

contract B2 is C {
	constructor(uint x) C(x + 2) {
		a = x;
	}
}

contract A is B2, B1 {
	constructor(uint x) B2(x) B1(x) {
		assert(a == x);
		assert(a == x + 1);
	}
}
// ----
// Warning 6328: (302-320): Assertion violation happens here
// Warning 2661: (200-205): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (200-205): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (314-319): Overflow (resulting value larger than 2**256 - 1) happens here
