pragma experimental SMTChecker;

abstract contract A {
	int x = 0;

	function f() public view mod() {
		assert(x != 0); // fails for A
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
// ----
// Warning 6328: (104-118): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nA.constructor()\nState: x = 0\nA.f()
// Warning 6328: (137-151): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n\n\nTransaction trace:\nB.constructor()\nState: x = 0\nA.f()
// Warning 6328: (170-184): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\n\n\nTransaction trace:\nC.constructor()\nState: x = 0\nA.f()
// Warning 6328: (203-217): CHC: Assertion violation happens here.\nCounterexample:\nx = 3\n\n\n\nTransaction trace:\nD.constructor()\nState: x = 0\nA.f()
