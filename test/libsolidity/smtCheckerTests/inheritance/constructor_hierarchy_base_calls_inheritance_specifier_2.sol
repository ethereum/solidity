pragma experimental SMTChecker;

contract A {
	uint public x;
	constructor(uint a) { x = a; }
}

contract B is A(9) {
	constructor(uint b) {
		x += b;
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
		assert(x == 15);
		assert(x > 90); // should fail
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 4984: (143-149): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (333-340): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (409-423): CHC: Assertion violation happens here.
