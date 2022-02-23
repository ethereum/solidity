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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4984: (110-116): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4984: (300-307): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (376-390): CHC: Assertion violation happens here.
