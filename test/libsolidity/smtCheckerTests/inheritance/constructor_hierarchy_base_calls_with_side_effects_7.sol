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

contract Z is B {
	constructor() B(f()) {
	}
}

contract C is Z {
	constructor() {
		assert(x == 1);
		assert(x > 2); // should fail
	}
}
// ----
// Warning 6328: (387-400): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n\n\nTransaction trace:\nconstructor()
