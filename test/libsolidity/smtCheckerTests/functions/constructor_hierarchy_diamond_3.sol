pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) {
		a = x;
	}
}

abstract contract B1 is C {
	uint b1;
	constructor(uint x) {
		b1 = x + a;
	}
}

contract B2 is C {
	uint b2;
	constructor(uint x) C(x + 2) {
		b2 = x + a;
	}
}

contract A is B2, B1 {
	constructor(uint x) B2(x) B1(x) {
		assert(b1 == b2);
		assert(b1 != b2);
	}
}
// ----
// Warning 4984: (160-165): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\nb1 = 0, a = 115792089237316195423570985008687907853269984665640564039457584007913129639935\nx = 1\n\n\nTransaction trace:\nconstructor(1)
// Warning 4984: (241-246): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\nb2 = 0, a = 1\nx = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n\n\nTransaction trace:\nconstructor(115792089237316195423570985008687907853269984665640564039457584007913129639935)
// Warning 4984: (225-230): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\nb2 = 0, a = 0\nx = 115792089237316195423570985008687907853269984665640564039457584007913129639934\n\n\nTransaction trace:\nconstructor(115792089237316195423570985008687907853269984665640564039457584007913129639934)
// Warning 6328: (334-350): CHC: Assertion violation happens here.
