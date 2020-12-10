pragma experimental SMTChecker;

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

abstract contract Z is A {
	uint k;
	constructor(uint z) {
		k = z;
	}
}

contract C is Z, B {
	constructor() Z(g()) B(f()) {
		assert(x == 1);
		assert(k == 42);
		assert(x == k); // should fail
	}
}
// ----
// Warning 6328: (449-463): CHC: Assertion violation happens here.\nCounterexample:\nk = 42, x = 1\n\n\n\nTransaction trace:\nconstructor()
