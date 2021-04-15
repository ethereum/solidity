abstract contract A {
	int x = 0;

	function f() public view mod() {
		assert(x != 0); // does not hold for A, but A is abstract so it should not be reported
		assert(x != 1); // fails for B
		assert(x != 2); // fails for C
		assert(x != 3); // fails for D
	}

	modifier mod() virtual {
		_;
	}
}

contract B is A {
	modifier mod() virtual override {
		x = 1;
		_;
	}
}

contract C is A {
	modifier mod() virtual override {
		x = 2;
		_;
	}
}

contract D is B,C {
	modifier mod() virtual override (B,C){
		x = 3;
		_;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (160-174): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\nTransaction trace:\nB.constructor()\nState: x = 0\nA.f()
// Warning 6328: (193-207): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nC.constructor()\nState: x = 0\nA.f()
// Warning 6328: (226-240): CHC: Assertion violation happens here.\nCounterexample:\nx = 3\n\nTransaction trace:\nD.constructor()\nState: x = 0\nA.f()
