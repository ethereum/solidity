pragma experimental SMTChecker;

// 2 warnings, fallback and A.g
contract A {
	uint x;

	fallback () external virtual {
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
		assert(x == 1);
	}
}
// ----
// Warning 6328: (122-136): Assertion violation happens here
// Warning 6328: (171-185): Assertion violation happens here
// Warning 6328: (288-302): Assertion violation happens here
