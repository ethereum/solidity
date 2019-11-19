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
	uint y;

	function f() public view override {
		assert(x == 0);
	}
}
// ----
// Warning: (121-135): Assertion violation happens here
// Warning: (170-184): Assertion violation happens here
// Warning: (286-300): Assertion violation happens here
// Warning: (170-184): Assertion violation happens here
