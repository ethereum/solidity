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
}

abstract contract Z is A {
	uint k;
	constructor(uint z) {
		k = z;
	}
}

contract C is Z, B {
	constructor() B(x) Z(x) {
		assert(x == 1);
		assert(k == 0);
		assert(x == k); // should fail
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 4984: (138-145): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (384-398): CHC: Assertion violation happens here.
