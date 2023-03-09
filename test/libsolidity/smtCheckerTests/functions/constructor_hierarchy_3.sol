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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4984: (171-176): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (200-218): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
