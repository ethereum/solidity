pragma experimental SMTChecker;

// 2 warnings, fallback and A.g
contract A {
	uint x;

	fallback () external {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 2 warnings, fallback and A.g
contract B is A {
	uint y;

	fallback () external override {
		assert(x == 0);
	}
}
// ----
// Warning: (114-128): Assertion violation happens here
// Warning: (163-177): Assertion violation happens here
// Warning: (280-294): Assertion violation happens here
// Warning: (163-177): Assertion violation happens here
