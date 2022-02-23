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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4984: (125-131): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (185-190): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (241-259): CHC: Assertion violation happens here.
