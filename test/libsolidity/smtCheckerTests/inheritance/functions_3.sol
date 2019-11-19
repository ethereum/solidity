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
		assert(x == 0);
	}
	function h() public view {
		assert(x == 2);
	}
}

// 4 warnings, C.f, C.i, B.h, A.g
contract C is B {
	uint z;

	function f() public view override {
		assert(x == 0);
	}
	function i() public view {
		assert(x == 0);
	}
}
// ----
// Warning: (121-135): Assertion violation happens here
// Warning: (170-184): Assertion violation happens here
// Warning: (296-310): Assertion violation happens here
// Warning: (345-359): Assertion violation happens here
// Warning: (170-184): Assertion violation happens here
// Warning: (468-482): Assertion violation happens here
// Warning: (517-531): Assertion violation happens here
// Warning: (345-359): Assertion violation happens here
// Warning: (170-184): Assertion violation happens here
