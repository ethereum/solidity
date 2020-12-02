pragma experimental SMTChecker;
contract F {
	uint a;
	constructor(uint x) {
		a = x;
	}
}

abstract contract E is F {}
abstract contract D is E {
	constructor() {
		a = 3;
	}
}
abstract contract C is D {}
contract B is C {
	constructor(uint x) F(x + 1) {
	}
}

contract A is B {
	constructor(uint x) B(x) {
		assert(a == 3);
		assert(a == 4);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 4984: (247-252): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (328-342): CHC: Assertion violation happens here.
