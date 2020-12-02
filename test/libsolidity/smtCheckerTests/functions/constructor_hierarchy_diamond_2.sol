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
	constructor(uint x) B1(x) B2(x) {
		assert(a == x);
		assert(a == x + 1);
	}
}
// ----
// Warning 4984: (200-205): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\na = 0\nx = 115792089237316195423570985008687907853269984665640564039457584007913129639934\n\n\nTransaction trace:\nconstructor(115792089237316195423570985008687907853269984665640564039457584007913129639934)
// Warning 4984: (314-319): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (302-320): CHC: Assertion violation happens here.\nCounterexample:\na = 0\nx = 0\n\n\nTransaction trace:\nconstructor(0)
