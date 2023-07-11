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
// ====
// SMTEngine: all
// ----
// Warning 4984: (166-171): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (175-180): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
