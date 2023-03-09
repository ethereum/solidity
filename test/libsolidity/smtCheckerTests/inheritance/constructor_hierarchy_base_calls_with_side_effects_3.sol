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
	constructor(uint c) B(c) Z(x) {
		assert(x == c + 1);
		assert(k == 0);
		assert(x == k); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4984: (105-112): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (361-375): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
