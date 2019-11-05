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

// 2 warnings, B.f and A.g
contract B is A {
	uint y;

	function f() public view override {
		assert(x == 0);
	}
}
// ----
// Warning: (113-127): Assertion violation happens here
// Warning: (162-176): Assertion violation happens here
// Warning: (278-292): Assertion violation happens here
// Warning: (162-176): Assertion violation happens here
