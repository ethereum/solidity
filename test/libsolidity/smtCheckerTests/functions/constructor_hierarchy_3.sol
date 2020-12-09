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
// ====
// SMTIgnoreCex: yes
// ----
// Warning 4984: (244-249): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (232-250): CHC: Assertion violation happens here.
// Warning 4984: (203-208): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
