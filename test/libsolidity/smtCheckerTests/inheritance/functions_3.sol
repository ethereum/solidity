pragma experimental SMTChecker;

// 2 warnings, A.f and A.g
contract A {
	uint x;

	function f() public view {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 3 warnings, B.f, B.h, A.g
contract B is A {
	uint y;

	function f() public view override {
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
// Warning: (113-127): Assertion violation happens here
// Warning: (162-176): Assertion violation happens here
// Warning: (280-294): Assertion violation happens here
// Warning: (329-343): Assertion violation happens here
// Warning: (162-176): Assertion violation happens here
// Warning: (452-466): Assertion violation happens here
// Warning: (501-515): Assertion violation happens here
// Warning: (329-343): Assertion violation happens here
// Warning: (162-176): Assertion violation happens here
