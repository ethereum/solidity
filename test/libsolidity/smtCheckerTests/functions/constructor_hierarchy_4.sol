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
	constructor(uint x) C(x + 2) B(x + 1) {
		assert(a == x + 1);
	}
}
// ----
// Warning 2661: (207-212): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (198-203): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (230-235): Overflow (resulting value larger than 2**256 - 1) happens here
