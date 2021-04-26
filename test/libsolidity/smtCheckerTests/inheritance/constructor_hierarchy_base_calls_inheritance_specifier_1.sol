contract A {
	uint public x;
	constructor(uint a) { x = a; }
}

contract B is A {
	constructor(uint b) A(b) {
	}

	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}

	function g() internal returns (uint) {
		x = 42;
		return x;
	}
}

contract Z is B {
	constructor(uint z) B(z + f()) {
	}
}

contract C is Z(5) {
	constructor() {
		assert(x == 6);
		assert(x > 9); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (292-299): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\nx = 1\nz = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n\nTransaction trace:\nZ.constructor(115792089237316195423570985008687907853269984665640564039457584007913129639935)
// Warning 6328: (367-380): CHC: Assertion violation happens here.\nCounterexample:\nx = 6\n\nTransaction trace:\nC.constructor()
