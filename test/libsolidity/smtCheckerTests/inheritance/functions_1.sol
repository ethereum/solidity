pragma experimental SMTChecker;

// 2 warnings, A.f and A.g
contract A {
	uint x;

	function f() public virtual view {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 2 warnings, B.f and A.g
contract B is A {
	function f() public view override {
		assert(x == 1);
	}
}
// ----
// Warning 6328: (121-135): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf()
// Warning 6328: (170-184): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng()
// Warning 6328: (276-290): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf()
