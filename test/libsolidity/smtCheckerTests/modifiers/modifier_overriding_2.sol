pragma experimental SMTChecker;

abstract contract A {
	bool s;

	function f() public view mod {
		assert(s); // holds for C, but fails for B
	}
	modifier mod() virtual;
}

contract B is A {
	modifier mod() virtual override {
		s = false;
		_;
	}
}

contract C is B {
	modifier mod() override {
		s = true;
		_;
	}
}
// ----
// Warning 5740: (95-144): Unreachable code.
// Warning 6328: (99-108): CHC: Assertion violation happens here.\nCounterexample:\ns = false\n\nTransaction trace:\nB.constructor()\nState: s = false\nA.f()
