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

	function f() public view {
		assert(x == 0);
	}
	function h() public view {
		assert(x == 2);
	}
}

// 4 warnings, C.f, C.i, B.h, A.g
contract C is B {
	uint z;

	function f() public view {
		assert(x == 0);
	}
	function i() public view {
		assert(x == 0);
	}
}
// ----
// Warning: (113-127): Assertion violation happens here
// Warning: (162-176): Assertion violation happens here
// Warning: (271-285): Assertion violation happens here
// Warning: (320-334): Assertion violation happens here
// Warning: (162-176): Assertion violation happens here
// Warning: (434-448): Assertion violation happens here
// Warning: (483-497): Assertion violation happens here
// Warning: (320-334): Assertion violation happens here
// Warning: (162-176): Assertion violation happens here
