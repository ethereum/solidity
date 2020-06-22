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
// Warning 4661: (121-135): Assertion violation happens here
// Warning 4661: (170-184): Assertion violation happens here
// Warning 4661: (296-310): Assertion violation happens here
// Warning 4661: (345-359): Assertion violation happens here
// Warning 4661: (170-184): Assertion violation happens here
// Warning 4661: (468-482): Assertion violation happens here
// Warning 4661: (517-531): Assertion violation happens here
// Warning 4661: (345-359): Assertion violation happens here
// Warning 4661: (170-184): Assertion violation happens here
