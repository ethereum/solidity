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
		b = x + 10;
	}
}

contract A is B {
	constructor(uint x) B(x) C(x + 2) {
		assert(a == x + 2);
		assert(b == x + 10);
		assert(b == x + 5);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 4984: (157-163): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (240-245): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (262-268): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (285-290): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (273-291): CHC: Assertion violation happens here.
// Warning 4984: (217-222): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
