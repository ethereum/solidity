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

// 3 warnings, B.f, B.h, A.g
contract B is A {
	uint y;

	function f() public view virtual override {
		assert(x == 1);
	}
	function h() public view {
		assert(x == 1);
	}
}

// 4 warnings, C.f, C.i, B.h, A.g
contract C is B {
	uint z;

	function f() public view override {
		assert(x == 1);
	}
	function i() public view {
		assert(x == 1);
	}
}
// ----
// Warning 6328: (121-135): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf()
// Warning 6328: (170-184): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng()
// Warning 6328: (296-310): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, x = 0\n\n\n\nTransaction trace:\nconstructor()\nState: y = 0, x = 0\nf()
// Warning 6328: (345-359): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, x = 0\n\n\n\nTransaction trace:\nconstructor()\nState: y = 0, x = 0\nh()
// Warning 6328: (468-482): CHC: Assertion violation happens here.\nCounterexample:\nz = 0, y = 0, x = 0\n\n\n\nTransaction trace:\nconstructor()\nState: z = 0, y = 0, x = 0\nf()
// Warning 6328: (517-531): CHC: Assertion violation happens here.\nCounterexample:\nz = 0, y = 0, x = 0\n\n\n\nTransaction trace:\nconstructor()\nState: z = 0, y = 0, x = 0\ni()
