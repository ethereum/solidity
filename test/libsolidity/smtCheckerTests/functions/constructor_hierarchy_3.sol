pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) {
		a = x;
	}
}

abstract contract B is C {
	constructor(uint x) {
		a = x;
	}
}

contract A is B {
	constructor(uint x) B(x) C(x + 2) {
		assert(a == x);
		assert(a == x + 1);
	}
}
// ----
// Warning 6328: (232-250): Assertion violation happens here
// Warning 2661: (203-208): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (244-249): Overflow (resulting value larger than 2**256 - 1) happens here
