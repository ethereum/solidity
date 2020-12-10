pragma experimental SMTChecker;

contract A {
	uint public x;
	constructor(uint a) { x = a; }
}

contract B is A {
	constructor(uint b) A(b + f()) {
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

abstract contract Z is A {
	uint k;
	constructor(uint z) {
		k = z;
	}
}

contract C is Z, B {
	constructor() Z(g()) B(f()) {
		assert(x == 44);
		assert(k == 42);
		assert(x == k); // should fail
	}
}
// ----
// Warning 4984: (138-145): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\nx = 1\nb = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n\n\nTransaction trace:\nconstructor(115792089237316195423570985008687907853269984665640564039457584007913129639935)
// Warning 6328: (456-470): CHC: Assertion violation happens here.
