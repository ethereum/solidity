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
// ====
// SMTIgnoreCex: yes
// ----
// Warning 4984: (157-162): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (239-244): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (261-266): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (261-270): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (287-292): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (275-293): CHC: Assertion violation happens here.
// Warning 4984: (216-221): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
